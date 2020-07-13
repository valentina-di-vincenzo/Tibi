/*
 * Copyright (c) 2020 Valentina Di Vincenzo ( hello@valentina-divincenzo.com )
 *  This file is part of Tibi which is released under the GNU General Public License, version 3.0.
 * See file LICENSE or go to http://www.gnu.org/licenses/ for full license details.
 *
 */

#include "UploadsDispatcher.h"

void UploadsDispatcher::setUserInfo(UserHandler* user) {
    this->user = user;
}

void UploadsDispatcher::newFileSelection(const QVector<QString>& filePaths) {
    qDebug() << "upload dispatcher: new file selection called";
    for( QString filePath : filePaths ) {
        selectedFilePathsAndNames.insert(filePath, "");
    }
    QVector<QString> fileNames;
    computeFileNames(fileNames);
    emit showConnectedTibiers(fileNames);

}


void UploadsDispatcher::tibiersSelectionReady(const QVector<QString>& selectedId) {

    if( selectedId.isEmpty() ) {
        clearSelectionSession();
        return;
    }//request cancelled

    QSharedPointer<QVector<Tibier>> selectedTibiers = user->getTibiersFromId(selectedId);
    if( selectedTibiers->isEmpty() ) {
        clearSelectionSession();
        return;
    }//no selected tibiers online anymore

    qDebug() << QThread::currentThreadId() << "Creating uploads handlers..";

    for( Tibier tibier : *selectedTibiers ) {
        QMap<QString, QString>::iterator fileName;
        for (fileName = selectedFilePathsAndNames.begin(); fileName != selectedFilePathsAndNames.end(); ++fileName) {

            emit addUploadToView(true, uploadCode, tibier.username, fileName.value());
            createUploadHandler(uploadCode, tibier, fileName.key());
            uploadCode++;
        }
    }

    clearSelectionSession();

}

void UploadsDispatcher::createUploadHandler(int code, Tibier& tibier, const QString& filePath) {
    QSharedPointer<Tibier> selectedTibier = QSharedPointer<Tibier>(new Tibier(tibier));
    QThread *uploadThread = new QThread;
    UploadHandler* handler = new UploadHandler(code, selectedTibier, filePath, user);

    handler->moveToThread(uploadThread);
    uploads.insert(code, handler);

    /* -- CLEAN UP -- */
    connect(handler, &UploadHandler::terminationRequest, uploadThread, &QThread::quit);
    connect(handler, &UploadHandler::terminationRequest, handler, &UploadHandler::deleteLater);
    connect(uploadThread, &QThread::finished, uploadThread, &QObject::deleteLater);


    /* -- START --*/
    connect(uploadThread, &QThread::started, handler, &UploadHandler::initSocket);

    connectUploadHandler(handler);

    uploadThread->start();
    emit addUploadToTray();

}

void UploadsDispatcher::connectUploadHandler(UploadHandler* handler) {
    connect(this, &UploadsDispatcher::cleanUp, handler, &UploadHandler::signalAbort, Qt::DirectConnection);
    connect(this, &UploadsDispatcher::cleanUp, handler, &UploadHandler::abortConnection);
    connect(handler, &UploadHandler::updateUpload, this, &UploadsDispatcher::updateUpload);
}

void UploadsDispatcher::abortFromView(int uploadCode) {
    UploadHandler* handler = uploads.value(uploadCode);
    handler->signalAbort();
    connect(this, &UploadsDispatcher::abortUpload, handler, &UploadHandler::abortConnection, Qt::QueuedConnection);
    emit abortUpload();
    disconnect(this, &UploadsDispatcher::abortUpload, handler, &UploadHandler::abortConnection);

}


void UploadsDispatcher::computeFileNames(QVector<QString>& fileNames) {

    QMap<QString, QString>::iterator pair;
    for (pair = selectedFilePathsAndNames.begin(); pair != selectedFilePathsAndNames.end(); ++pair) {
        QFileInfo file(pair.key());
        pair.value() = file.fileName();
        fileNames.push_back(file.fileName());
    }

}

void UploadsDispatcher::updateUpload(QSharedPointer<Upload> upload) {
    QTime t(0,0,0);
    QString info;
    int perc = 100;

    if( upload->status == Status::Error ) {
        info = "ERROR";
        emit removeUploadFromTray();
    }

    else if( upload->status == Status::WaitingForAnswer) {
        perc = 0;
        info = "WAITING";
    }

    else if( upload->status == Status::Declined ) {
        info = "DECLINED";
        emit removeUploadFromTray();
    }

    else if( upload->status == Status::Aborted ) {
        info = "ABORTED";
        emit removeUploadFromTray();
    }

    else if ( upload->status == Status::Completed) {
        info = "COMPLETED";
        emit removeUploadFromTray();
    }

    else if( upload->status == Status::Failed) {
        info = "FAILED";
        emit removeUploadFromTray();
    }

    else if( upload->status == Status::Accepted ) {
        perc =  static_cast<int>(upload->totalByteSent*100/upload->totalSize);
        qint64 byteLeft = upload->totalSize - upload->totalByteSent;
        qint64 elapsedTime = upload->timeStart.elapsed();
        qint64 uploadTimeMsecs = (byteLeft * elapsedTime) / upload->totalByteSent;
        t = t.addMSecs(uploadTimeMsecs);
    }

    /*qDebug() << "Upload mediator emits: "
             << "\nperc: " << perc
             << "\ninfo: " << info;*/
    emit updateRowUpload(true, upload->code, perc, t, info);

}



void UploadsDispatcher::clearSelectionSession() {
    selectedFilePathsAndNames.clear();
    emit endSelectionSession();
}



UploadsDispatcher::~UploadsDispatcher() {
    emit cleanUp();
    qDebug() << "Uploads mediator clean";
}
