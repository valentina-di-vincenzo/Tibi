/*
 * Copyright (c) 2020 Valentina Di Vincenzo ( hello@valentina-divincenzo.com )
 *  This file is part of Tibi which is released under the GNU General Public License, version 3.0.
 * See file LICENSE or go to http://www.gnu.org/licenses/ for full license details.
 *
 */

#include "DownloadsDispatcher.h"

void DownloadsDispatcher::setUserInfo(UserHandler* user) {
    this->user = user;
}


void DownloadsDispatcher::createDownloadHandler(qintptr socketDescriptor) {

    checkDownloadDirectoryExistance();
    QThread *handlerThread = new QThread;
    DownloadHandler* handler = new DownloadHandler(socketDescriptor, downloadsCode, user->getDownloadPath());

    handler->moveToThread(handlerThread);

    downloads.insert(downloadsCode, handler);
    downloadsCode++;

    /* -- CONNECT -- */

    connect(this, &DownloadsDispatcher::cleanUp, handler, &DownloadHandler::appClose);
    connect(handler, &DownloadHandler::updateDownload, this,  &DownloadsDispatcher::updateDownload);
    connect(handler, &DownloadHandler::askOverwrite, this, &DownloadsDispatcher::askOverwrite, Qt::BlockingQueuedConnection);
    connect(handler, &DownloadHandler::informCollision, this, &DownloadsDispatcher::informCollision, Qt::BlockingQueuedConnection);
    connect(handler, &DownloadHandler::terminationRequest, handlerThread, &QThread::quit);
    connect(handler, &DownloadHandler::terminationRequest, handler, &DownloadHandler::deleteLater);
    connect(handlerThread, &QThread::finished, handlerThread, &QObject::deleteLater);


    /* -- START --*/
    connect(handlerThread, &QThread::started, handler, &DownloadHandler::initSocket);


    connect(handler, &DownloadHandler::waitForConfirmation, this, &DownloadsDispatcher::computeAnswer);
    connect(handler, &DownloadHandler::downloadFinished, this, &DownloadsDispatcher::downloadFinished, Qt::BlockingQueuedConnection);
    handlerThread->start();

}


void DownloadsDispatcher::abortFromView(int downloadCode) {
    DownloadHandler* handler = downloads.value(downloadCode);
    handler->signalAbort();
    connect(this, &DownloadsDispatcher::abortDownload, handler, &DownloadHandler::abortConnection, Qt::QueuedConnection);
    emit abortDownload();
    connect(this, &DownloadsDispatcher::abortDownload, handler, &DownloadHandler::abortConnection);
}

void DownloadsDispatcher::computeAnswer(QSharedPointer<Download> download) {
    qDebug() << "DISPATCHER: download handler is waiting for an answer";
    if( user->getConfirmation() ) {
        manageConfrimationView(download);
    }

    else {
        newAnswer(download, Status::Accepted);
        emit addDownloadToTray();
        emit notifyUserDownloadStatus(download);
    }

}

void DownloadsDispatcher::manageConfrimationView(QSharedPointer<Download> download) {
    conf = new ViewConfirmation(*download);
    timer = new QTimer;
    timer->setSingleShot(true);
    connect(conf, &ViewConfirmation::userAnswer, this, [=](bool accepted) {
        accepted? newAnswer(download, Status::Accepted) : newAnswer(download, Status::Declined);
    });
    connect(conf, &ViewConfirmation::userAnswer, conf, &ViewConfirmation::deleteLater);
    connect(conf, &ViewConfirmation::userAnswer, this, [=]() {
        conf = nullptr;
    });
    connect(conf, &ViewConfirmation::userAnswer, timer, &QTimer::stop);
    connect(timer, &QTimer::timeout, timer, &QTimer::deleteLater);
    connect(timer, &QTimer::timeout, this, [=]() {
        newAnswer(download, Status::Declined);
        conf->deleteLater();
        conf = nullptr;
    });

    timer->start(150*1000);
    conf->showConfirmation(user->getLockFromId(download->idSender));

}

void DownloadsDispatcher::newAnswer(QSharedPointer<Download> download, Status status) {
    int code = download->code;
    if(!downloads.contains(code)) return;
    if( status == Status::Accepted ) {
        emit addDownloadToView(false, code, download->usernameSender, download->itemName);
    }
    connect(this, &DownloadsDispatcher::userAnswer, downloads.value(code), &DownloadHandler::userAnswer);
    emit userAnswer(code, status);
    disconnect(this, &DownloadsDispatcher::userAnswer, downloads.value(code), &DownloadHandler::userAnswer);
}

void DownloadsDispatcher::downloadFinished(QSharedPointer<Download> download) {
    qDebug() << "Mediator: download terminated"
             << "\nDownload final state: " << download->status;
    if( download->status != Status::Declined) {
        emit notifyUserDownloadStatus(download);
    }
    downloads.remove(download->code);
    //if the user close the app while the confirmation window is open
    if( conf!=nullptr && conf->code == download->code ) {
        timer->deleteLater();
        conf->deleteLater();
        conf = nullptr;
    }
}

bool DownloadsDispatcher::askOverwrite(int code, const QString& type, const QString &oldName, const QString &newName) {
    QMessageBox msgBox;
    QPixmap pixmapLogo(":/images/tibi-icon.png");
    pixmapLogo = pixmapLogo.scaled(50, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    msgBox.setIconPixmap(pixmapLogo);
    msgBox.setText("A " + type +" called " + oldName + " already exists. \nDo you want to overwrite it?");
    msgBox.setInformativeText("If you don't, It will be saved as " + newName);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();
    if( ret == QMessageBox::No ) {
        emit updateDownloadName(code, newName);
        return false;
    }
   return true;
}

void DownloadsDispatcher::informCollision(int code, const QString& oldName, const QString& newName) {
    QMessageBox::information(nullptr, "DOWNLOAD FILE PROBLEM", "You are currently downloading a file with the same name '" + oldName + "'. The file will be saved as " + newName);
    emit updateDownloadName(code, newName);
}
void DownloadsDispatcher::checkDownloadDirectoryExistance() {
    if(QDir(user->getDownloadPath()).exists()) return;

    user->setDownloadPath(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
    QMessageBox::critical(nullptr, "DOWNLOAD DIRECTORY", "The selected directory doesn't exist anymore. Downloads will be saved in " + user->getDownloadPath() + ". Change download directory in Preferences.");


}

void DownloadsDispatcher::updateDownload(QSharedPointer<Download> download) {
    QTime t(0,0,0);
    QString info;
    int perc = 100;

    if( download->status == Status::Error ) {
        info = "ERROR";
    }

    else if( download->status == Status::Aborted || download->status == Status::AbortedFromUser) {
        info = "ABORTED";
    }

    else if ( download->status == Status::Completed) {
        info = "COMPLETED";
    }

    else if( download->status == Status::Failed) {
        info = "FAILED";
    }

    else if( download->status == Status::Accepted ) {
        qint64 byteReceived = download->totalSize - download->totalBytesLeft;
        perc =  static_cast<int>(byteReceived*100/download->totalSize);
        qint64 elapsedTime = download->timeStart.elapsed();
        qint64 downloadTimeMsecs = (download->totalBytesLeft * elapsedTime) / byteReceived;
        t = t.addMSecs(downloadTimeMsecs);
    }

    /*qDebug() << "Download mediator emits: "
             << "\nperc: " << perc
             << "\ninfo: " << info;*/
    if( download->status == Status::Completed ) {
        emit updateRowDownload(false, download->code, perc, t, info, download->downloadsPath + "/" + download->itemName);
    }
    else {
        emit updateRowDownload(false, download->code, perc, t, info);
    }
}

DownloadsDispatcher::~DownloadsDispatcher() {
    emit cleanUp();

}
