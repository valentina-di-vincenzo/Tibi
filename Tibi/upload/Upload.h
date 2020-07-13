/*
 * Copyright (c) 2020 Valentina Di Vincenzo ( hello@valentina-divincenzo.com )
 *  This file is part of Tibi which is released under the GNU General Public License, version 3.0.
 * See file LICENSE or go to http://www.gnu.org/licenses/ for full license details.
 *
 */

#ifndef UPLOADS_H
#define UPLOADS_H
#include "user/tibier.h"
#include "TypesAndStatus.h"
#include <QTime>


class Upload
{
public:
    Upload() {}
    int code;
    QSharedPointer<Tibier> tibierReceiver;
    QString itemName;
    QTime timeStart = QTime(0,0,0);
    qint64 totalSize = 0;
    qint64 totalByteSent = 0;
    Status status;

};

Q_DECLARE_METATYPE(Upload)

#endif // UPLOADS_H
