/*
 * Copyright (c) 2020 Valentina Di Vincenzo ( hello@valentina-divincenzo.com )
 *  This file is part of Tibi which is released under the GNU General Public License, version 3.0.
 * See file LICENSE or go to http://www.gnu.org/licenses/ for full license details.
 *
 */

#include "TibiDiscovery.h"

void TibiDiscovery::setUserInfo(UserHandler* user) {
    this->user = user;
    qDebug() << QThread::currentThreadId() << " - TIBI DISCOVERY start discovery";

}

void TibiDiscovery::startDiscovery() {
    tibierPing = QSharedPointer<Tibier>(new Tibier);
    avatarArrayPing = QSharedPointer<QByteArray>(new QByteArray);
    groupAddress4.setAddress("224.0.0.1");
    udpSocket4 = new QUdpSocket();
    udpSocket4->bind(QHostAddress::AnyIPv4, 45454, QUdpSocket::ShareAddress);
    udpSocket4->joinMulticastGroup(groupAddress4);
    connect(udpSocket4, &QUdpSocket::readyRead,
            this, &TibiDiscovery::processPendingDatagrams);
}



void TibiDiscovery::processPendingDatagrams() {

    while (udpSocket4->hasPendingDatagrams()) {

        QNetworkDatagram dgram = udpSocket4->receiveDatagram();
        data = new QByteArray;
        *data = dgram.data();
        ping = new QDataStream(data, QIODevice::ReadOnly);


        if( checkLoopback() ) continue;
        readPing();
        if(tibierPing->avatarCode != 0 && user->checkIfNewAvatarTibier(tibierPing->id, tibierPing->avatarCode)) {
            createNewAvatarArray();
        }
        user->newPing(tibierPing, avatarArrayPing);
        tibierPing.reset(new Tibier);
        avatarArrayPing.reset(new QByteArray);
        delete ping;
        ping = nullptr;
        delete data;
        data = nullptr;
        /*
        qDebug() << "Received PING from " <<  dgram.senderAddress().toString()
                 << "@" << dgram.senderPort()
                 << "size: " << dgram.data().size()
                 << "\nPING: "
                 << "\n id: " << tibierPing->id.toString()
                 << "\n username: " << tibierPing->username
                 << "\n address: " << tibierPing->address
                 << "\n port: " << tibierPing->port
                 << "\n avatar code: " << tibierPing->avatarCode;
*/
    }
}

bool TibiDiscovery::checkLoopback() {
    *ping >> tibierPing->id;

    if( tibierPing->id == user->getId() ) {
        return true;
    }
    tibierPing->updateAvatarPath();
    return false;

}
void TibiDiscovery::readPing() {

    *ping >> tibierPing->username;
    QString addressPing;
    *ping >> addressPing;
    tibierPing->address = QHostAddress(addressPing);
    *ping >> tibierPing->port;
    *ping >> tibierPing->avatarCode;
    tibierPing->online = true;
    tibierPing->lastPing = QDateTime::currentSecsSinceEpoch();

    //qDebug() << QThread::currentThreadId() << " - TIBI DISCOVERY ping read";

}

void TibiDiscovery::createNewAvatarArray() {

    *ping >> avatarSizePing;
    for( int i=0; i< avatarSizePing; i++) {
        qint8 byte;
        *ping >> byte;
        avatarArrayPing->append(byte);
    }

    qDebug() << QThread::currentThreadId() << " - TIBI DISCOVERY creato nuovo avatar array";

}


void TibiDiscovery::closeSocket() {
    if( ping != nullptr) delete ping;
    if( data != nullptr) delete data;
    if( udpSocket4 != nullptr) delete udpSocket4;
    emit finished();
}


