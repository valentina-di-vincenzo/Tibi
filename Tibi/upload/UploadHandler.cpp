/*
 * Copyright (c) 2020 Valentina Di Vincenzo ( hello@valentina-divincenzo.com )
 *  This file is part of Tibi which is released under the GNU General Public License, version 3.0.
 * See file LICENSE or go to http://www.gnu.org/licenses/ for full license details.
 *
 */

#include "UploadHandler.h"


UploadHandler::UploadHandler(int code, QSharedPointer<Tibier> tibier, const QString& itemPath, UserHandler* user)

{
    qDebug() << QThread::currentThreadId() << " - TIBI SENDER constructor";
    upload.code = code;
    upload.tibierReceiver = tibier;
    this->itemPath = itemPath;
    this->user = user;

}


void UploadHandler::initSocket() {

    qDebug() << "[SENDER " << upload.code << " - " << QThread::currentThreadId() << " ] New upload request " << itemPath
             << " to " << upload.tibierReceiver->username
             << " on " << upload.tibierReceiver->address.toString()
             << "@" << upload.tibierReceiver->port;

    senderSocket = new QSslSocket;
    setSocketConfig();
    connectSocket();
    senderSocket->connectToHostEncrypted(upload.tibierReceiver->address.toString(), upload.tibierReceiver->port);

}


void UploadHandler::setSocketConfig() {
    QSslConfiguration config = senderSocket->sslConfiguration();
    config.setPeerVerifyMode(QSslSocket::VerifyNone);
    senderSocket->setSslConfiguration(config);
    senderSocket->ignoreSslErrors();
    senderSocket->addCaCertificates(":/certificates/tibi.pem");
}


void UploadHandler::connectSocket() {
    connect(senderSocket, &QSslSocket::connected, this, &UploadHandler::connected);
    connect(senderSocket, &QSslSocket::readyRead, this, &UploadHandler::getAnswer);
    connect(senderSocket, &QSslSocket::disconnected, senderSocket, &QSslSocket::deleteLater);
    connect(senderSocket, &QSslSocket::encrypted, this, [=]() {
        qDebug() << "\nencrypted? " << senderSocket->isEncrypted()
                 << "\ncipher: " << senderSocket->sessionCipher().name()
                 << "\nprotcol: " << senderSocket->sessionCipher().protocolString();
    });
    connect(senderSocket, static_cast<void ( QTcpSocket::* )( QAbstractSocket::SocketError )>( &QAbstractSocket::error ), this,
            [=]( QAbstractSocket::SocketError ) {

        if( senderSocket->error() == QAbstractSocket::RemoteHostClosedError ) {
            qDebug() << "Remote host close the connection";
            if( upload.status != Status::Declined ) {
                signalStatusAndTerminate(Status::Failed, "The receiver closed the connection."); }
            else {
                signalStatusAndTerminate(Status::Declined, "The receiver " + upload.tibierReceiver->username + " declined " + itemPath);}
        } else {
            qDebug() << "Error in socket";
            signalStatusAndTerminate(Status::Error, senderSocket->errorString());
        }
    });

    connect(senderSocket, QOverload<const QList<QSslError> &>::of(&QSslSocket::sslErrors),
            [=](const QList<QSslError> &err){
        QString error;
        for( auto errMsg : err ) {
            error += "\n ERROR SSL: " + errMsg.errorString();
            qDebug() << error;
        }
    });
}



void UploadHandler::connected()
{
    qDebug() << "Sender connected to " << upload.tibierReceiver->username
             << " at " << senderSocket->peerAddress()
             << "@" << senderSocket->peerPort();


    initOut();
    prepareFirstMetadata();
    sendFirstMetadata();
    qDebug() << "The sender sent the selected " << (isDir? " dir ":" file ")
             << "'s info. Waiting for an answer.";
    upload.status = Status::WaitingForAnswer;
    emit updateUpload(QSharedPointer<Upload>(new Upload(upload)));

}


void UploadHandler::initOut() {
    out = new QDataStream(&outBlock, QIODevice::WriteOnly);
    out->setVersion(QDataStream::Qt_5_5);
}


void UploadHandler::prepareFirstMetadata() {
    qDebug() << "Preparing first metadata";
    itemInfo.setFile(itemPath);
    if( !checkItemExistance() ) return;
    fillItemName();
    fillItemType();
    fillItemSize();
    qDebug() << "item name: " << upload.itemName
             << "\nitem type: " << isDir
             << "\nitem size: " << upload.totalSize;
}


bool UploadHandler::checkItemExistance() {
    if( !itemInfo.exists() ) {
        signalStatusAndTerminate(Status::Error, "The selected file doesn't exist");
        return false;
    }
    return true;
}


void UploadHandler::fillItemName() {
    upload.itemName = itemInfo.fileName();
}


void UploadHandler::fillItemType() {
    if( itemInfo.isDir() ) {
        isDir = Types::Dir;
    } else {
        isDir = Types::File;
    }
}


void UploadHandler::fillItemSize() {
    if( isDir ) {
        fillDirInfo();
    } else {
        upload.totalSize = itemInfo.size();
    }

    qDebug() << "UPLOAD SIZE: " << upload.totalSize;
}


void UploadHandler::fillDirInfo() {
    dirToSend.setPath(itemPath);
    upload.totalSize = getDirSize(itemPath);
}



qint64 UploadHandler::getDirSize(const QString& path) {
    QDir dir(path);
    qint64 size = sumFileSizeInDir(dir);
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden);
    // if there are no child -> the analyzed directory is a leaf
    // it's relative path is added to the dirLeavesRelativePaths vector
    if( dir.entryInfoList().isEmpty() && dir != dirToSend) {
        QString relativePath = dirToSend.relativeFilePath(dir.path());
        qDebug() << "sub dir relative path: " << relativePath;
        dirLeavesRelativePaths.push_back(relativePath);
        return size;
    }

    for ( auto childDir : dir.entryInfoList()) {
        qint64 sizeChild = getDirSize(childDir.filePath());
        size += sizeChild;
    }
    return size;
}


qint64 UploadHandler::sumFileSizeInDir(QDir& dir) {
    qint64 sizeFiles = 0;

    dir.setFilter(QDir::Files | QDir::Hidden);
    for( auto file : dir.entryInfoList() ) {
        // each path is added to the filePaths vector
        qDebug() << "file: " << file.filePath();
        filePaths.push_back(file.filePath());
        sizeFiles += file.size();
    }

    return sizeFiles;
}


void UploadHandler::sendFirstMetadata() {

    if( checkAbort() ) return;
    QString username = user->getUsername();
    QString itemName = upload.itemName;
    quint8 usernameSize = static_cast<quint8>(username.size());
    quint8 itemNameSize = static_cast<quint8>(itemName.size());
    quint16 metadataSize = static_cast<quint16>(16 + 1 + 1 + 2 + 8 + usernameSize + itemNameSize);
    qDebug() << "metadata size: " << metadataSize;
    *out << metadataSize;
    *out << user->getId();
    *out << username;
    *out << static_cast<quint8>(isDir ? 1 : 0);
    *out << itemName;
    *out << upload.totalSize;
    writeBlock();
}


bool UploadHandler::checkAbort() {
    QMutexLocker ml(&abort_m);

    if( abort ) {
        if( !abortAlreadyCalled ) {
            abortConnection();
            abortAlreadyCalled = true;
        }
    }

    return abort;
}



void UploadHandler::getAnswer() {

    qDebug() << "Client receives answer of : " << senderSocket->bytesAvailable() << " bytes";

    QDataStream in;
    in.setDevice(senderSocket);
    in.setVersion(QDataStream::Qt_5_5);

    if( senderSocket->bytesAvailable() == sizeof(bool) ) {
        bool declined;
        in >> declined;
        qDebug() << "Answer: " << (declined ? "refuse" : "accept")
                 << " \t BYTE RESTANTI: " << senderSocket->bytesAvailable();

        declined ? refused() : accepted();

    }

}


void UploadHandler::refused() {
    upload.status = Status::Declined;
}


void UploadHandler::accepted() {
    upload.status = Status::Accepted;
    upload.timeStart.start();
    isDir == Types::Dir ? sendDir() : sendFile(itemPath);
    if( checkAbort() ) return;
    qDebug() << "All bytes are sent. Wait for the receiver to disconnect";
    upload.status = Status::Completed;
    emit updateUpload(QSharedPointer<Upload>(new Upload(upload)));
}


void UploadHandler::sendFile(const QString& absolutePath) {
    if( checkAbort() ) return;
    QFile file(absolutePath);
    if( !openFile(file) ) return;
    if( isDir ) sendFileInfo(absolutePath, file);

    qDebug() << "File absolute path " << absolutePath;

    currentFilebytesLeft = file.size();
    qint64 sizeToRead = 12288;
    while (currentFilebytesLeft > 0 && !checkAbort()) {
        if( currentFilebytesLeft < 12288 ) sizeToRead = currentFilebytesLeft;
        qDebug() << "size to read: " << sizeToRead;
        outBlock.insert(0, file.read(sizeToRead));
        qint64 x = sizeToRead;
        while( x > 0 ) {
            x -= senderSocket->write(outBlock);
            qDebug() << "byteLeft: " << x << "/" << sizeToRead;
        }
        currentFilebytesLeft -= sizeToRead;
        upload.totalByteSent += sizeToRead;
        // at each iteration, a copy of the current status is shared with the DownloadDispatcher
        outBlock.clear();
        emit updateUpload(QSharedPointer<Upload>(new Upload(upload)));
    }

    senderSocket->waitForBytesWritten();

}

void UploadHandler::sendFileInfo(const QString &absolutePath, const QFile &file) {

    QString relativePath = dirToSend.relativeFilePath(absolutePath);
    qDebug() << "File relative path: " << relativePath;
    qDebug() << "sent path size: " << relativePath.size();
    qDebug() << "file size: " << file.size();
    *out << static_cast<quint16>(relativePath.size());
    *out << relativePath;
    *out << file.size();
    writeBlock();
}


bool UploadHandler::openFile(QFile &file) {
    if( !file.open(QIODevice::ReadOnly)) {
        signalStatusAndTerminate(Status::Error, "Impossible to open the file");
        return false;
    }
    return true;
}




void UploadHandler::sendDir() {
    // 1. send subdirs ( subdir counter + subdirs infos )
    if( checkAbort() ) return;
    sendSubDirs();

    // 2. send files counter
    *out << static_cast<qint16>(filePaths.count());
    writeBlock();
    qDebug() << "files counter: " << filePaths.count();

    // 3. send each file
    for( const QString& filePath : filePaths ) {
        sendFile(filePath);
    }


}


void UploadHandler::sendSubDirs() {
    *out << static_cast<qint16>(dirLeavesRelativePaths.count());
    qDebug() << "sub dir counter: " << dirLeavesRelativePaths.count();

    for( const QString& leaf : dirLeavesRelativePaths ) {
        *out << static_cast<quint16>(leaf.size());
        *out << leaf;
        qDebug() << "sending leaf: " << leaf
                 << "\nsize path: " << leaf.size();
    }
    writeBlock();
}

void UploadHandler::writeBlock() {
    out->device()->seek(0);
    senderSocket->write(outBlock);
    senderSocket->waitForBytesWritten();
    outBlock.clear();
}


void UploadHandler::signalAbort() {
    QMutexLocker ml(&abort_m);
    abort = true;
}

void UploadHandler::abortConnection() {
    if( abortAlreadyCalled ) return;
    qDebug() << QThread::currentThreadId() <<
                " ] The user aborted";
    senderSocket->abort();
    signalStatusAndTerminate(Status::Aborted, "Abort requested from user");
}

void UploadHandler::signalStatusAndTerminate(Status status, const QString& message) {
    senderSocket->disconnect();
    qDebug() << "NEW UPLOAD STATUS " << upload.code << " - " << status << " : " << message;

    if( status == Status::Failed ) {
        abort = true;
    }

    else if( senderSocket->state() != QAbstractSocket::UnconnectedState && senderSocket->state() !=  QAbstractSocket::ClosingState ) {
        senderSocket->disconnectFromHost();
    }

    if( out != nullptr) {
        delete out;
        out = nullptr;
    }

    if( upload.status != Status::Completed ) {
        upload.status = status;
        emit updateUpload(QSharedPointer<Upload>(new Upload(upload)));
    }

    qDebug() << "emits Termination Request with status: " << upload.status;
    emit terminationRequest();

}

UploadHandler::~UploadHandler() {
    qDebug() << "Destroying sender associated with " << itemPath;
}
