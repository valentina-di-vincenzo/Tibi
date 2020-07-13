/*
 * Copyright (c) 2020 Valentina Di Vincenzo ( hello@valentina-divincenzo.com )
 *  This file is part of Tibi which is released under the GNU General Public License, version 3.0.
 * See file LICENSE or go to http://www.gnu.org/licenses/ for full license details.
 *
 */

#ifndef TIBIER_H
#define TIBIER_H
#include <QUuid>
#include <QHostAddress>
#include <QString>
#include <QDir>
#include <QReadWriteLock>
#include <QStandardPaths>
#include <QSharedPointer>

class Tibier {

public:
    Tibier() {
        avatar_m = QSharedPointer<QReadWriteLock>(new QReadWriteLock);
    }

    QUuid id;
    QHostAddress address;
    bool online;
    int port = 0;
    QString username;
    qint64 lastPing = 0;
    qint8 avatarCode = 0;
    QString avatarPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/avatars/" + id.toString() + ".jpg";
    bool operator ==(const Tibier& t);
    QSharedPointer<QReadWriteLock> avatar_m;
    void updateAvatarPath() {
        avatarPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/avatars/" + id.toString() + ".jpg";
    }


};

Q_DECLARE_METATYPE(Tibier)

#endif // TIBIER_H
