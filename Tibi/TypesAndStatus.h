/*
 * Copyright (c) 2020 Valentina Di Vincenzo ( hello@valentina-divincenzo.com )
 *  This file is part of Tibi which is released under the GNU General Public License, version 3.0.
 * See file LICENSE or go to http://www.gnu.org/licenses/ for full license details.
 *
 */

#ifndef TYPES_H
#define TYPES_H
#include <QObject>

enum Types {
    File = 0,
    Dir = 1,
    Unknown = 2
};

enum Status {
    Declined = 0,
    Accepted = 1,
    Aborted = 2,
    Completed = 3,
    Error = 4,
    Failed = 5,
    WaitingForAnswer = 6,
    AbortedFromUser = 7
};

Q_DECLARE_METATYPE(Status)

#endif // TYPES_H
