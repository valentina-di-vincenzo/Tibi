/*
 * Copyright (c) 2020 Valentina Di Vincenzo ( hello@valentina-divincenzo.com )
 *  This file is part of Tibi which is released under the GNU General Public License, version 3.0.
 * See file LICENSE or go to http://www.gnu.org/licenses/ for full license details.
 *
 */

#include "UDRow.h"

UDRow::UDRow(int code, QString tibierName, QString fileName) :
    code(code),
    to(new QLabel(tibierName)),
    name(new QLabel(fileName)) {

    bar = new QProgressBar;
    bar->setRange(0, 100);
    bar->setTextVisible(false);
    bar->setFixedWidth(210);

    time = new QLabel("elaborating..");
    time->setFixedWidth(150);
    abort = new QPushButton("abort");
    abort->setObjectName("abort");
    abort->setFixedWidth(70);

    to->setFixedWidth(150);
    name->setMinimumWidth(150);

}

UDRow::~UDRow() {
    delete to;
    delete name;
    delete time;
    delete bar;
    delete abort;
    if( open!= nullptr ) delete open;
}

