/*
 * Copyright (c) 2020 Valentina Di Vincenzo ( hello@valentina-divincenzo.com )
 *  This file is part of Tibi which is released under the GNU General Public License, version 3.0.
 * See file LICENSE or go to http://www.gnu.org/licenses/ for full license details.
 *
 */

#include "download/DownloadHandler.h"


DownloadHandler::DownloadHandler(qintptr socketDescriptor, int code, QString downloadsPath) :
    socketDescriptor(socketDescriptor)

{

    download = QSharedPointer<Download>( new Download );
    download->downloadsPath = downloadsPath;
    download->code = code;
    qDebug() << QThread::currentThreadId() << " - TIBI DOWNLOAD HANDLER constructor";

}

void DownloadHandler::initSocket() {

    qDebug() << QThread::currentThreadId() << " - TIBI DOWNLOAD HANDLER run";

    receiverSocket = new QSslSocket;
    configSocket();
    receiverSocket->setSocketDescriptor(socketDescriptor);

    in.setDevice(receiverSocket);
    in.setVersion(QDataStream::Qt_5_5);

    qDebug() << "Tibier connected: " << receiverSocket->peerAddress().toString()
             << "@" << receiverSocket->peerPort();

    connectSocket();
    receiverSocket->startServerEncryption();
    onReadyRead();

}

void DownloadHandler::configSocket() {
    QSslConfiguration config = receiverSocket->sslConfiguration();
    config.setPeerVerifyMode(QSslSocket::VerifyNone);
    receiverSocket->setSslConfiguration(config);
    receiverSocket->ignoreSslErrors();
    receiverSocket->setPrivateKey(":/certificates/certificates/tibi.key", QSsl::Rsa);
    receiverSocket->setLocalCertificate(":/certificates/certificates/tibi.pem");
}

void DownloadHandler::connectSocket() {

    /* -- Internal --*/
    connect(receiverSocket, &QSslSocket::readyRead, this, &DownloadHandler::onReadyRead, Qt::QueuedConnection);
    connect(receiverSocket, &QSslSocket::disconnected, receiverSocket, &QSslSocket::deleteLater);
    connect(receiverSocket, static_cast<void ( QTcpSocket::* )( QAbstractSocket::SocketError )>( &QAbstractSocket::error ), this,
            [=]( QAbstractSocket::SocketError ) {
        removeDownloads();
        if( receiverSocket->error() == QAbstractSocket::RemoteHostClosedError ) {
            signalStatusAndTerminate(Status::Aborted, "Abort requested from sender");
        } else {
            signalStatusAndTerminate(Status::Error, receiverSocket->errorString());
        }
    });

    connect(receiverSocket, QOverload<const QList<QSslError> &>::of(&QSslSocket::sslErrors),
            [=](const QList<QSslError> &err){
        QString error;
        for( auto errMsg : err ) {
            error += "\n ERROR SSL: " + errMsg.errorString();
            qDebug() << error;
        }
    });



}


void DownloadHandler::onReadyRead() {
    if( receiverSocket->bytesAvailable() == 0 ) return;
    if( checkAbort()) return;

    if( !metadataCompleted ) {
        metadataCompleted = getFirstMetadata();
    } else {
        download->type == Types::Dir ? downloadDir() : downloadFile();
    }

}

bool DownloadHandler::getFirstMetadata() {
    qDebug() << "get first metadata called";
    if( firstMetadataSize == 0 && receiverSocket->bytesAvailable() >= 2 ) {
        qDebug() << "firstMetadataSize: " << firstMetadataSize;
        in >> firstMetadataSize;
    }

    if( firstMetadataSize!=0 && receiverSocket->bytesAvailable() >= firstMetadataSize) {
        getmetadata();
        qDebug() << "download handler emits wait for confirmation signal";
        emit waitForConfirmation(download);
        return true;
    }

    return false;

}

void DownloadHandler::getmetadata() {
    qDebug() << "Received metadata of " << receiverSocket->bytesAvailable() << "bytes";
    in >> download->idSender;
    in >> download->usernameSender;
    getType();
    in >> download->itemName;
    in >> download->totalSize;
    download->totalBytesLeft = download->totalSize;

    qDebug() << "id sender: " << download->idSender.toString()
             << "\nusername sender: " << download->usernameSender
             << "\ntype: " << download->type
             << "\nitem name: " << download->itemName
             << "\nitem size: " << download->totalSize
             << "\n(" << receiverSocket->bytesAvailable() << " left in the socket )";
}

void DownloadHandler::getType() {
    quint8 type_code = 3;
    in >> type_code;
    if( type_code == 0 ) download->type = Types::File;
    else if( type_code == 1 ) download->type = Types::Dir;
    else {
        signalStatusAndTerminate(Status::Error, "Received unknown type");

    }
}


void DownloadHandler::userAnswer(int code, Status status) {
    qDebug() << "handler: user answer received";
    if( code != download->code ) return;
    download->status = status;
    download->status == Status::Accepted ?
                accepted() :
                declined();

}

void DownloadHandler::declined() {
    sendAnswerToSender();
    signalStatusAndTerminate(Status::Declined, "User declined the confirmation request");
}

void DownloadHandler::accepted() {
    qDebug() << "User confirmed the download. Start downloading the content of the first metadata";
    if( download->type == Types::Dir) {
        computeDirDownloadPath();
        dirToDownload.setPath(itemPath);
        QDir downloadsDir(download->downloadsPath);
        if( !downloadsDir.mkpath(download->itemName) ) { signalStatusAndTerminate(Status::Error, "Impossible to create directory " + itemPath);
            return;
        }
        qDebug() << "dir created";
    } else {
        computeFileDownloadPath();
        relativePath.clear();
        if( ! openFile() ) return;
        fileSize = download->totalSize;
        bytesLeft = fileSize;
        filesCounter = 1;
        subDirCounter = 0;
        qDebug() << "Ready to wait for file";
    }

    sendAnswerToSender();
    download->timeStart.start();

}


bool DownloadHandler::openFile() {
    qDebug() << "open file at: " << itemPath + "/" + relativePath;
    if( download->type == Types::Dir ) currentDownload = new QFile(itemPath + "/" + relativePath);
    else currentDownload = new QFile(itemPath);


    if( ! currentDownload->open(QIODevice::WriteOnly) ) {
        if( currentDownload != nullptr)  {
            delete currentDownload;
            currentDownload = nullptr;
        }

        signalStatusAndTerminate(Status::Error, "Impossible to open file");
        return false;
    }

    return true;
}

void DownloadHandler::sendAnswerToSender() {
    QByteArray answerBlock;
    QDataStream out(&answerBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_5);
    download->status == Status::Declined ?
                out << true : out << false;
    receiverSocket->write(answerBlock);
    qDebug() << "Request" << (download->status == Status::Accepted ? "accepted" : "declined");
    receiverSocket->waitForBytesWritten();
    qDebug() << "Answer sent";
}


void DownloadHandler::computeDirDownloadPath() {
    // since this is the first item, we need to check if other directory have the same name

    dirToDownload.setPath(download->downloadsPath + "/" + download->itemName);

    QString dirName = "";
    for(int i = 1; dirToDownload.exists(); i++) {
        dirName = download->itemName + " (" + QString::number(i) + ")";
        dirToDownload.setPath(download->downloadsPath + "/" + dirName);
    }

    newItemName(dirName);

    // check if other threads are downloading dir with the same name
    if( concurrentCollision() ) return;
    itemPath = download->downloadsPath + "/" + download->itemName;
    qDebug() << "final item path: " << itemPath;
}


void DownloadHandler::computeFileDownloadPath() {
    QString newName = "";
    QString clearFileName = getClearFileName(download->itemName);
    QString fileExtension = getFileExtension(download->itemName);
    QFileInfo infoFile(download->downloadsPath + "/" + download->itemName);

    //1. check if the file already exists
    for( int i=1; infoFile.exists(); i++) {
        newName = clearFileName + " (" + QString::number(i) + ")" + fileExtension;
        infoFile.setFile(download->downloadsPath + "/" + newName);
    }

    newItemName(newName);
    if( concurrentCollision() ) return;
    itemPath = download->downloadsPath + "/" + download->itemName;
    qDebug() << "final item path: " << itemPath;


}

void DownloadHandler::newItemName(const QString &newName) {
    //if it already exists, ask the user what to do
    if( newName != "") {
        QString typeString = "directory";
        if( download->type != Types::Dir )  typeString = "file";
        bool overwrite = emit askOverwrite(download->code, typeString, download->itemName, newName);
        if( !overwrite ) {
            download->itemName = newName;
        }
    }

}


bool DownloadHandler::concurrentCollision(const QString& fileExtension) {
    //check if other threads are currently downloading on the same path
    fileLock = new QLockFile(download->downloadsPath + "/" + download->itemName + "lockfile");
    fileLock->setStaleLockTime(0);
    if( !fileLock->tryLock() ) {
        delete fileLock;
        QString collision = download->itemName;
        if( fileExtension == nullptr) {
            download->itemName += " copy";
        }
        else {
            download->itemName = getClearFileName(download->itemName) + " copy" + fileExtension;
        }
        emit informCollision(download->code, collision, download->itemName);
        computeFileDownloadPath();
        return true;
    }
    return false;
}



void DownloadHandler::downloadDir() {
    qDebug() << "\n\nByte nel receiver: " << receiverSocket->bytesAvailable();
    //1. Create subdirs first
    getCounter(subDirCounter);
    qDebug() << "sub dir counter: " << subDirCounter;
    if( subDirCounter > 0 ) {
        createSubDir();
    }

    //2. when subDirCounter is zero, all the sub dirs are created. Proceede to download files.
    if( subDirCounter == 0 ) {
        getCounter(filesCounter);
        qDebug() << "File counter: " << filesCounter;
        qDebug() << "pathSize: " << pathSize;

        if( filesCounter > 0 ) {
            if( !infoFileCompleted ) {
                infoFileCompleted = getFileInfo();
                return;
            }
            downloadFile();
        }
    }
}



void DownloadHandler::getCounter(qint16 &counter) {
    if( counter < 0 && receiverSocket->bytesAvailable() >= 2) {
        in >> counter;
        qDebug() << "NEW COUNTER: " << counter;
    }

}


void DownloadHandler::createSubDir() {
    if( !getRelativePath() ) return;
    if( !dirToDownload.mkpath(relativePath) ) {
        signalStatusAndTerminate(Status::Error, "Impossible to create directory " + download->itemName + relativePath);
        return;
    }
    subDirCounter--;
    qDebug() << "sub dir left: " << subDirCounter;
    pathSize = 0;
    relativePath.clear();
    if( subDirCounter == 0 ) checkEnd();
}

bool DownloadHandler::getFileInfo() {
    qDebug() << "get info file: ";
    getRelativePath();
    if( relativePath != "" && receiverSocket->bytesAvailable() >= 8 ) {
        in >> fileSize;
        bytesLeft = fileSize;
        qDebug() << "file size: " << fileSize;
        if( !openFile() ) return false;
        return true;
    }

    return false;

}

bool DownloadHandler::getRelativePath() {
    getPathSize();
    if( pathSize == 0 || relativePath != "" || receiverSocket->bytesAvailable() < static_cast<qint64>(pathSize*sizeof(QChar))) return false;

    in >> relativePath;
    qDebug() << "relative path: " << relativePath;
    return true;

}

void DownloadHandler::getPathSize() {
    if( pathSize == 0 && receiverSocket->bytesAvailable() >= 2) {
        in >> pathSize;
        qDebug() << "relative path size: " << pathSize;
    }
}


void DownloadHandler::downloadFile() {

    qDebug() << "\nDownload file " << itemPath + "/" + relativePath << "\n byte left: " << bytesLeft << "/" << fileSize;
    qDebug() << "Byte to read in the receiver: " << receiverSocket->bytesAvailable();


    qint64 written = 0;
    if( written < fileSize ) {
        written += currentDownload->write(receiverSocket->read(bytesLeft));
        qDebug() << "Byte written in the file: " << written;
    }
    bytesLeft -= written;
    download->totalBytesLeft -= written;

    emit updateDownload(QSharedPointer<Download>(new Download(*download)));

    if( bytesLeft > 0 ) return;

    if( currentDownload != nullptr)  {
        delete currentDownload;
        currentDownload = nullptr;
    }

    pathSize = 0;
    relativePath.clear();
    fileSize = 0;
    bytesLeft = 0;
    infoFileCompleted = false;

    filesCounter--;
    qDebug() << "File left: " << filesCounter;
    if( filesCounter == 0 ) checkEnd();

}


void DownloadHandler::checkEnd() {
    qDebug() << "Check end: Byte restanti da leggere: " << download->totalBytesLeft;
    qDebug() << "Byte nel receiver: " << receiverSocket->bytesAvailable();

    if(download->totalBytesLeft != 0) return;
    signalStatusAndTerminate(Status::Completed, "All bytes received");

}


QString DownloadHandler::getClearFileName(const QString& relativePath) {

    QStringRef name(&relativePath, 0, relativePath.lastIndexOf("."));
    return name.toString();
}

QString DownloadHandler::getFileExtension(const QString& relativePath) {
    int estensioneSize = relativePath.size() - relativePath.lastIndexOf(".");
    QStringRef estensione(&relativePath, relativePath.lastIndexOf("."), estensioneSize);
    return estensione.toString();
}

void DownloadHandler::appClose() {
    removeDownloads();
    signalStatusAndTerminate(Status::Aborted, "Abort requested");
}

void DownloadHandler::signalAbort() {
    QMutexLocker ml(&abort_m);
    abort = true;
}

void DownloadHandler::abortConnection() {
    if( abortAlreadyCalled ) return;
    removeDownloads();
    qDebug() << QThread::currentThreadId() <<
                " ] The user aborted";
    receiverSocket->abort();
    signalStatusAndTerminate(Status::AbortedFromUser, "Abort requested from user");
}

bool DownloadHandler::checkAbort() {
    QMutexLocker ml(&abort_m);

    if( abort ) {
        if( !abortAlreadyCalled ) {
            abortConnection();
            abortAlreadyCalled = true;
        }
    }

    return abort;
}

void DownloadHandler::removeDownloads() {
    qDebug() << "The sender aborted the operation or the application has to exit. Removing data. ";
    if(itemPath == "") return; //abort request happened in the initial phase. nothing to remove
    qDebug() << "Removing " << itemPath;
    if(download->type == Types::Dir ) {
        dirToDownload.removeRecursively();
    } else {
        currentDownload->remove();
    }

    if(fileLock != nullptr) {
        delete fileLock;
        fileLock = nullptr;
    }

}

void DownloadHandler::signalStatusAndTerminate(Status status, const QString& message) {
    qDebug() << "NEW DOWNLOAD STATUS of " << download->itemName << " : " << message;
    receiverSocket->disconnect();
    download->status = status;
    if( receiverSocket->state() == QAbstractSocket::ConnectedState ) {
        receiverSocket->disconnectFromHost();
    }

    emit downloadFinished(download);
    if(download->status != Status::Declined) emit updateDownload(QSharedPointer<Download>(new Download(*download)));

    if(fileLock != nullptr) {
        delete fileLock;
        fileLock = nullptr;
    }

    if( currentDownload != nullptr)  {
        delete currentDownload;
        currentDownload = nullptr;
    }


    emit terminationRequest();

}
