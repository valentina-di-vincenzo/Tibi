/*
 * Copyright (c) 2020 Valentina Di Vincenzo ( hello@valentina-divincenzo.com )
 *  This file is part of Tibi which is released under the GNU General Public License, version 3.0.
 * See file LICENSE or go to http://www.gnu.org/licenses/ for full license details.
 *
 */

#include "TibiMediator.h"

TibiMediator::TibiMediator()
{

    // to make types available to queued signal and slot connections
    registerMetatype();

    // QLocalSocket and QTcpSocket cannot be moved if they have a parent
    receiver = new TibiReceiver;
    selector = new TibiSelector;

    userHandler = UserHandler::Instance();
    userHandler->createUser();

    mediateDownload();
    mediateUpload();
    mediatePreferences();
    mediateTrayActions();
    mediateCleaning();

    start();

}

void TibiMediator::start() {
    connect(receiver, &TibiReceiver::receiverRunningAt, userHandler, &UserHandler::setReceiverAddress, Qt::DirectConnection);
    connect(userHandler, &UserHandler::userReady, this, &TibiMediator::dispatchUserInfo);
    startReceiverThread();
}

void TibiMediator::registerMetatype() {
    qRegisterMetaType<Tibier>();
    qRegisterMetaType<QVector<QString>>("QVector<QString>");
    qRegisterMetaType<QSharedPointer<Download>>("QSharedPointer<Download>");
    qRegisterMetaType<QSharedPointer<Upload>>("QSharedPointer<Upload>");
    qRegisterMetaType<qintptr>("qintptr");
    qRegisterMetaType<Status>();
}

/************************************************
 * -------- MEDIATOR OPERATIONS --------------- *
 ***********************************************/

void TibiMediator::mediateDownload() {
    connect(receiver, &TibiReceiver::newDownloadRequest, &downloadsDispatcher, &DownloadsDispatcher::createDownloadHandler);
    connect(&downloadsDispatcher, &DownloadsDispatcher::addDownloadToTray, &viewTray, &ViewTray::addDownload);
    connect(&downloadsDispatcher, &DownloadsDispatcher::addDownloadToView, &viewUD, &ViewUD::addUD);
    connect(&downloadsDispatcher, &DownloadsDispatcher::updateDownloadName, &viewUD, &ViewUD::updateDownloadName);
    connect(&downloadsDispatcher, &DownloadsDispatcher::updateRowDownload, &viewUD, &ViewUD::updateUD);
    connect(&downloadsDispatcher, &DownloadsDispatcher::notifyUserDownloadStatus, &viewTray, &ViewTray::notifyUserDownloadStatus);
    connect(&viewUD, &ViewUD::signalDownloadAbort, &downloadsDispatcher, &DownloadsDispatcher::abortFromView);
}

void TibiMediator::mediateUpload() {
    connect(selector, &TibiSelector::newFileSelection, &uploadsDispatcher, &UploadsDispatcher::newFileSelection);
    connect(&uploadsDispatcher, &UploadsDispatcher::showConnectedTibiers, &viewConnected, &ViewConnectedTibiers::showConnectedTibiers);
    connect(userHandler, &UserHandler::updateConnectedTibiers, &viewConnected, &ViewConnectedTibiers::update, Qt::QueuedConnection);
    connect(userHandler, &UserHandler::removeTibierFromSelected, &viewConnected, &ViewConnectedTibiers::removeTibierFromSelected);
    connect(&viewConnected, &ViewConnectedTibiers::selectionReady, &uploadsDispatcher, &UploadsDispatcher::tibiersSelectionReady);
    connect(&uploadsDispatcher, &UploadsDispatcher::addUploadToView, &viewUD, &ViewUD::addUD, Qt::QueuedConnection);
    connect(&uploadsDispatcher, &UploadsDispatcher::addUploadToTray, &viewTray, &ViewTray::addUpload);
    connect(&uploadsDispatcher, &UploadsDispatcher::endSelectionSession, selector, &TibiSelector::endSelectionSession);
    connect(&uploadsDispatcher, &UploadsDispatcher::updateRowUpload, &viewUD, &ViewUD::updateUD, Qt::QueuedConnection);
    connect(&viewUD, &ViewUD::signalUploadAbort, &uploadsDispatcher, &UploadsDispatcher::abortFromView);
    connect(&uploadsDispatcher, &UploadsDispatcher::removeUploadFromTray, &viewTray, &ViewTray::removeUpload);

}

void TibiMediator::mediatePreferences() {
    connect(&viewTray, &ViewTray::openPreferences, &viewPreferences, &ViewPreferences::showPreferences);
    // propagate changes:
    connect(userHandler, &UserHandler::updateAvatarPing, &ping, &TibiPing::updateAvatar, Qt::BlockingQueuedConnection); // blocking to grant mutex protection
    connect(userHandler, &UserHandler::updateStatus, &ping, &TibiPing::updateStatus);
    connect(userHandler, &UserHandler::updateStatus, &viewTray, &ViewTray::updateStatusAction);
}

void TibiMediator::mediateTrayActions() {
    connect(&viewTray, &ViewTray::raiseUploads, &viewUD, &ViewUD::raiseView);
    connect(&viewTray, &ViewTray::closeApplication, qApp, &QCoreApplication::quit);
}

void TibiMediator::mediateCleaning() {
    connect(this, &TibiMediator::cleanUp, selector, &TibiSelector::closeSelector, Qt::BlockingQueuedConnection);
    connect(this, &TibiMediator::cleanUp, receiver, &TibiReceiver::closeReceiver, Qt::BlockingQueuedConnection);
    connect(this, &TibiMediator::cleanUp, &ping, &TibiPing::closeSocket, Qt::BlockingQueuedConnection);
    connect(this, &TibiMediator::cleanUp, &discovery, &TibiDiscovery::closeSocket, Qt::BlockingQueuedConnection);

}


/************************************************
 * -------- THREADS INITIALIZATION ------------ *
 ***********************************************/

void TibiMediator::dispatchUserInfo() {
    viewTray.setUserInfo(userHandler);
    ping.setUserInfo(userHandler);
    uploadsDispatcher.setUserInfo(userHandler);
    discovery.setUserInfo(userHandler);
    viewConnected.setUserInfo(userHandler);
    viewPreferences.setUserInfo(userHandler);
    downloadsDispatcher.setUserInfo(userHandler);

    startDiscoveryThread();
    startPingThread();
    startSelectorThread();

}

void TibiMediator::startSelectorThread() {
    QThread* selectorThread = new QThread;
    selector->moveToThread(selectorThread);
    connect(selector, &TibiSelector::selectorClosed, selectorThread, &QThread::quit);
    connect(selector, &TibiSelector::selectorClosed, selector, &QObject::deleteLater);
    connect(selectorThread, &QThread::finished, selectorThread, &QThread::deleteLater);
    connect(selectorThread, &QThread::started, selector, &TibiSelector::startSelector);
    selectorThread->start();
}


void TibiMediator::startReceiverThread() {
    QThread* receiverThread = new QThread;
    receiver->moveToThread(receiverThread);
    connect(receiver, &TibiReceiver::receiverClosed, receiverThread, &QThread::quit);
    connect(receiver, &TibiReceiver::receiverClosed, receiver, &QObject::deleteLater);
    connect(receiverThread, &QThread::finished, receiverThread, &QThread::deleteLater);
    connect(receiverThread, &QThread::started, receiver, &TibiReceiver::startReceiver);
    receiverThread->start();

}



void TibiMediator::startPingThread() {
    QThread* pingThread = new QThread;
    ping.moveToThread(pingThread);
    connect(pingThread, &QThread::started, &ping, &TibiPing::startPing);
    connect(&ping, &TibiPing::finished, pingThread, &QThread::quit);
    connect(pingThread, &QThread::finished, pingThread, &QThread::deleteLater);
    pingThread->start();
}


void TibiMediator::startDiscoveryThread() {
    QThread* discoveryThread = new QThread;
    discovery.moveToThread(discoveryThread);
    connect(discoveryThread, &QThread::started, &discovery, &TibiDiscovery::startDiscovery);
    connect(&discovery, &TibiDiscovery::finished, discoveryThread, &QThread::quit);
    connect(discoveryThread, &QThread::finished, discoveryThread, &QThread::deleteLater);
    discoveryThread->start();
}


TibiMediator::~TibiMediator() {
    emit cleanUp();
}
