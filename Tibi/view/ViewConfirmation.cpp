/*
 * Copyright (c) 2020 Valentina Di Vincenzo ( hello@valentina-divincenzo.com )
 *  This file is part of Tibi which is released under the GNU General Public License, version 3.0.
 * See file LICENSE or go to http://www.gnu.org/licenses/ for full license details.
 *
 */

#include "ViewConfirmation.h"

ViewConfirmation::ViewConfirmation(const Download& download) : download(download)
{
    setWindowTitle("NEW DOWNLOAD REQUEST");

    setFixedWidth(450);
    setMinimumHeight(200);
    code = download.code;

}

void ViewConfirmation::showConfirmation(QSharedPointer<QReadWriteLock> avatar_m) {
    qDebug() << "Confirmation View called";
    QString titleText = download.usernameSender + "\nwants to send you a " + (download.type == Types::Dir? "folder" : "file") + "!";
    if( avatar_m.isNull() ) titleText.append("\nCAREFUL: this tibier is in hidden mode!");

    QLabel* title = new QLabel(titleText);
    title->setFixedWidth(300);
    title->setWordWrap(true);
    title->setObjectName("title-conf");

    QString infoText = "Name:\t" + download.itemName + "\nSize: \t" + computeSize();

    QLabel* info = new QLabel(infoText);
    info->setFixedWidth(300);
    info->setWordWrap(true);
    if( !avatar_m.isNull() ) avatar_m.data()->lockForWrite();
    ClickableLabel *avatar = new ClickableLabel(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/avatars/" + download.idSender.toString() + ".jpg", true, true);
    if( !avatar_m.isNull() ) avatar_m.data()->unlock();

    QPushButton* acceptButton = new QPushButton("accept");
    connect(acceptButton, &QPushButton::clicked, this, [=]() {
        hide();
        emit userAnswer(true);
    });
    QPushButton* declineButton = new QPushButton("decline");
    connect(declineButton, &QPushButton::clicked, this, [=]() {
        hide();
        emit userAnswer(false);
    });

    QHBoxLayout* buttonsLayout = new QHBoxLayout;
    buttonsLayout->addSpacing(150);
    buttonsLayout->setSpacing(10);
    buttonsLayout->addWidget(declineButton);
    buttonsLayout->addWidget(acceptButton);

    QGridLayout* layout = new QGridLayout;
    layout->addWidget(avatar, 0, 0, 2, 0);
    layout->addWidget(title, 0, 1);
    layout->addWidget(info, 1, 1);
    layout->setSpacing(10);

    QVBoxLayout* layoutV = new QVBoxLayout;
    layoutV->addLayout(layout);
    layoutV->addLayout(buttonsLayout);   
    layoutV->setSpacing(20);
    setLayout(layoutV);
    this->show();
    this->raise();

}

QString ViewConfirmation::computeSize() {
    qDebug() << download.totalSize;
    double gb = download.totalSize/1000000000.0;
    if( gb > 0.99 ) {
        return QString::number(gb) + " GB";
    }
    double mb = download.totalSize/1000000.0;
    if( mb > 0.99 ) {
        qDebug() << mb;
        return QString::number(mb) + " MB";
    }
    double kb = download.totalSize/1000.0;
    if( kb > 0.99 ) {
        return QString::number(kb) + " kB";
    }
    return QString::number(download.totalSize) + " bytes";
}

void ViewConfirmation::closeEvent(QCloseEvent *event) {

#ifdef Q_OS_OSX
    if (!event->spontaneous() || !isVisible()) {
        return;
    }
#endif
        event->ignore();
}


