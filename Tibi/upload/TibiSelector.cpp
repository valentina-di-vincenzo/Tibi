/*
 * Copyright (c) 2020 Valentina Di Vincenzo ( hello@valentina-divincenzo.com )
 *  This file is part of Tibi which is released under the GNU General Public License, version 3.0.
 * See file LICENSE or go to http://www.gnu.org/licenses/ for full license details.
 *
 */

#include "TibiSelector.h"

void TibiSelector::startSelector() {
    QString serverName = "selector";
    QLocalServer::removeServer(serverName);
    listen(serverName);
    qDebug() << "The local server is running";
}

void TibiSelector::incomingConnection(quintptr socketDescriptor) {
    qDebug() << "new selector connection";
    selectorSocket = new QLocalSocket();
    selectorSocket->setSocketDescriptor(static_cast<qintptr>(socketDescriptor));
    connect(selectorSocket, &QLocalSocket::disconnected, selectorSocket, &QLocalSocket::deleteLater);
    in.setDevice(selectorSocket);

    // Receiveing new selected files
    selectorSocket->waitForReadyRead();

    int selected;
    in >> selected;
    qDebug() << "The user wants to send " << selected << " files";

    for( int i=0; i < selected; i++ ) {
        QString selectedFile;
        in >> selectedFile;
        selectedFiles.push_back(selectedFile);
        qDebug() << i << " ) " << selectedFile;
    }

    emit newFileSelection(selectedFiles);

}

void TibiSelector::endSelectionSession() {
    selectedFiles.clear();
    selectorSocket->disconnectFromServer();
}

void TibiSelector::closeSelector() {
    emit selectorClosed();
}








