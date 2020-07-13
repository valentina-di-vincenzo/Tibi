/*
 * Copyright (c) 2020 Valentina Di Vincenzo ( hello@valentina-divincenzo.com )
 *  This file is part of Tibi which is released under the GNU General Public License, version 3.0.
 * See file LICENSE or go to http://www.gnu.org/licenses/ for full license details.
 *
 */

#include "TibiPing.h"

void TibiPing::setUserInfo(UserHandler* user) {
    this->user = user;
    id = user->getId();
    address = user->getAddress().toString();
    port = user->getPort();
    avatarPath = user->getAvatarPath();
    avatarCode = user->getAvatarCode();
}

void TibiPing::startPing() {
    qDebug() << QThread::currentThreadId() << " - TIBI PING start sending";

    groupAddress4.setAddress("224.0.0.1");
    udpSocket4 = new QUdpSocket;
    connect(this, &TibiPing::finished, udpSocket4, &QUdpSocket::deleteLater);
    udpSocket4->bind(QHostAddress(QHostAddress::AnyIPv4), 0);
    udpSocket4->setSocketOption(QAbstractSocket::MulticastTtlOption, 5);
    qDebug() << "Ready to multicast ping to group " << groupAddress4.toString() << "@" << "45454";

    updateAvatar(user->getAvatarCode());
    timer = new QTimer;
    connect(this, &TibiPing::finished, timer, &QTimer::deleteLater);
    connect(timer, &QTimer::timeout, this, &TibiPing::sendDatagram);

    if( user->getStatus() ) {
        timer->start(TIME_PING);
    }


    qDebug()
            << "PING: "
            << "\n id: " << id.toString()
            << "\n username: " << user->getUsername()
            << "\n address: " << address
            << "\n port: " << port
            << "\n avatar code: " << user->getAvatarCode();


}

bool TibiPing::updateAvatar(qint8 code) {
    avatarCode = code;
    if(avatarCode == 0 ) return true; //default avatar
    QFile file(avatarPath + ".jpg");
    avatarArray.clear();
    if( ! file.open(QIODevice::ReadOnly) ) {
        avatarCode = 0;
        return false;
    }
    avatarArray.insert(0, file.readAll());
    qDebug() << "Avatar array size: " << avatarArray.size();
    return true;

}

void TibiPing::sendDatagram() {
    //qDebug() << "\nNow sending ping ";

    QByteArray datagram;
    QDataStream ping(&datagram, QIODevice::WriteOnly);

    ping << id;
    ping << user->getUsername();
    ping << address;
    ping << port;
    ping << avatarCode;
    if( avatarCode != 0 ) {
        ping << avatarArray.size();
        for( qint8 byte : avatarArray ) {
            ping << byte;
        }
    }
    /*
    qDebug()
    << "\nPING size: " << datagram.size()
    << "\n id: " << user.id.toString()
    << "\n username: " << user.username
    << "\n address: " << user.address.toString()
    << "\n port: " << user.port;

*/
    udpSocket4->writeDatagram(datagram, groupAddress4, 45454);

    //qDebug() << "Bytes sent: " << sent;

}


void TibiPing::updateStatus(bool online) {
    if (!online) {
        timer->stop();
    } else {
        timer->start(TIME_PING);
    }
}

void TibiPing::closeSocket() {
    emit finished();
}

