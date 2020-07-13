/*
 * Copyright (c) 2020 Valentina Di Vincenzo ( hello@valentina-divincenzo.com )
 *  This file is part of Tibi which is released under the GNU General Public License, version 3.0.
 * See file LICENSE or go to http://www.gnu.org/licenses/ for full license details.
 *
 */

#include "TibiReceiver.h"


#include <stdlib.h>

void TibiReceiver::startReceiver() {
    QHostAddress ipAddress = getIpAddress();
    listen(ipAddress);
    emit receiverRunningAt(serverAddress(), serverPort());

    qDebug() << "Receiver listening on IP:" << serverAddress().toString()
             << "@" << serverPort();
}

QHostAddress TibiReceiver::getIpAddress() {
    QHostAddress ipAddress;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // use the first non-localhost IPv4 address
    for ( auto address: ipAddressesList ) {
        if ( address != QHostAddress::LocalHost &&
                address.toIPv4Address()) {
            ipAddress = address;
            break;
        }
    }
    return ipAddress;
}

void TibiReceiver::incomingConnection(qintptr socketDescriptor)
{
    qDebug() << "TIBI RECEIVER: A Tibier wants to send something!";

    emit newDownloadRequest(socketDescriptor);

}

void TibiReceiver::closeReceiver() {
    close();
    emit receiverClosed();
}


