/*
 * Copyright (c) 2020 Valentina Di Vincenzo ( hello@valentina-divincenzo.com )
 *  This file is part of Tibi which is released under the GNU General Public License, version 3.0.
 * See file LICENSE or go to http://www.gnu.org/licenses/ for full license details.
 *
 */

#ifndef TIBIDISCOVERY_H
#define TIBIDISCOVERY_H

#include <QHostAddress>
#include <QUdpSocket>
#include <QtNetwork>
#include <QUuid>
#include "user/UserHandler.h"


/**
 * @brief The TibiDiscovery class, listen for TibiPing. If the avatar code is different from the one stored, the new avatar is downloaded.
 */

class TibiDiscovery : public QObject
{
    Q_OBJECT

public:
    TibiDiscovery() {}
    void setUserInfo(UserHandler* user);

public slots:

    void startDiscovery();
    void closeSocket();

signals:
    void finished();

private:
    UserHandler* user = nullptr;

    /* -- NETWORK --*/
    QUdpSocket* udpSocket4 = nullptr;
    QHostAddress groupAddress4;
    QDataStream* ping = nullptr;
    QByteArray* data = nullptr;

    /* -- PING RECEIVED --*/
    QSharedPointer<Tibier> tibierPing;
    int avatarSizePing = 0;
    QSharedPointer<QByteArray> avatarArrayPing;

    /* -- METHODS --*/
    void readPing();
    bool checkLoopback();
    void checkIfNewAvatar();
    bool checkIfNewTibier();
    void createNewAvatarArray();

private slots:
    void processPendingDatagrams();

};

#endif // TIBIDISCOVERY_H
