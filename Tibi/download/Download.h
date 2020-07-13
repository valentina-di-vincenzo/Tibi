/*
 * Copyright (c) 2020 Valentina Di Vincenzo ( hello@valentina-divincenzo.com )
 *  This file is part of Tibi which is released under the GNU General Public License, version 3.0.
 * See file LICENSE or go to http://www.gnu.org/licenses/ for full license details.
 *
 */

#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include "user/tibier.h"
#include "TypesAndStatus.h"
#include <QString>
#include <QObject>
#include <QUuid>
#include <QTime>

/**
 * @brief The Download class
 * Object useed as a request to update the ViewUD. It encapsuletes a unique code to identify the specific download request in the DownloadDispatcher and ViewUD. This object contains the essential information usefull to the other modules.
 */
class Download {
public:
    Download() {}
    int code = 0;
    QString usernameSender = "";
    QUuid idSender;
    qint64 totalBytesLeft = 0;
    qint64 totalSize = 0;
    QString itemName;
    QString downloadsPath;
    Types type;
    QTime timeStart = QTime(0,0,0);
    Status status = Status::WaitingForAnswer;
    bool operator ==(const Download& d) {
        if( this->code == d.code ) {
            return true;
        } else {
            return false;
        }
    }

};

Q_DECLARE_METATYPE(Download)

#endif // DOWNLOAD_H
