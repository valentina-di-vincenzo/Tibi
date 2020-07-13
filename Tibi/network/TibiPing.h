/*
 * Copyright (c) 2020 Valentina Di Vincenzo ( hello@valentina-divincenzo.com )
 *  This file is part of Tibi which is released under the GNU General Public License, version 3.0.
 * See file LICENSE or go to http://www.gnu.org/licenses/ for full license details.
 *
 */

#ifndef TIBIPING_H
#define TIBIPING_H

#include <QObject>
#include <QtNetwork>
#include <QTimer>
#include <QUuid>
#include "user/UserHandler.h"

#define TIME_PING 5000

/**
 * @brief The TibiPing class, if online sends a ping every 5 seconds.
 * Ping structure:
 * ID           QUuid - 128 bit - 16-bytes
 * USERNAME     QString
 * ADDRESS      QString
 * PORT         int
 * AVATAR CODE  qint8
 * AVATAR SIZE  qint64
 * ABATAR ARRAY qByteArray
 */

class TibiPing : public QObject
{
    Q_OBJECT

public:
    TibiPing() {}
    void setUserInfo(UserHandler* user);

public slots:
    void startPing();
    void updateStatus(bool online);
    bool updateAvatar(qint8 code);
    void closeSocket();

signals:
    void error(const QString& object, const QString& error);
    void finished();


private:
    /* --- NETWORK ATTRIBUTES --*/
    QUdpSocket* udpSocket4 = nullptr;
    QHostAddress groupAddress4;

    /* --- PING INFO -- */
    UserHandler* user = nullptr;
        // immutable
    QUuid id;
    QString address;
    int port;
        // mutable
    qint8 avatarCode;
    QString avatarPath;
    QByteArray avatarArray;

    /* --- TIMER FOR PING -- */
    QTimer* timer = nullptr;

private slots:
    void sendDatagram();

};

#endif // TIBIPING_H
