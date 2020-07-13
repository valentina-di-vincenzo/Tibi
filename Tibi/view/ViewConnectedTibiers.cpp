/*
 * Copyright (c) 2020 Valentina Di Vincenzo ( hello@valentina-divincenzo.com )
 *  This file is part of Tibi which is released under the GNU General Public License, version 3.0.
 * See file LICENSE or go to http://www.gnu.org/licenses/ for full license details.
 *
 */

#include "ViewConnectedTibiers.h"

ViewConnectedTibiers::ViewConnectedTibiers()
{

    createDialog();

    this->setFixedSize(500,420);
    this->hide();


}

void ViewConnectedTibiers::setUserInfo(UserHandler* user) {
    this->user = user;
}

void ViewConnectedTibiers::closeEvent(QCloseEvent *event)
{

#ifdef Q_OS_OSX
    if (!event->spontaneous() || !isVisible()) {
        return;
    }
#endif

    hide();
    event->ignore();
    cancelSelectionRequest();
}

void ViewConnectedTibiers::showConnectedTibiers(const QVector<QString>& fileNames) {
    qDebug() << "connected tibier called";
    int size = fileNames.size();
    QString info = "Share";
    if( size == 1 ) info += " '" + fileNames[0] + "'";
    else info += " " + QString::number(size) + " items";
    info += " with..";
    infoText.setText(info);
    infoText.setMaximumWidth(width()-20);
    infoText.setWordWrap(true);
    active = true;
    update();

    this->show();
    this->raise();
}

void ViewConnectedTibiers::update() {
    if( !active ) return;
    QSharedPointer<QVector<Tibier>> currentlyConnected = user->readCurrentlyConnectedTibiers();

    initCentralWidget();

    for( Tibier t : *currentlyConnected ) {
        qDebug() << "VIEW CONNECTED: analizzo tibier: " << t.username;
        if(!t.online) continue;
        QLabel* username = new QLabel(t.username);
        username->setAlignment(Qt::AlignCenter);
        username->setStyleSheet("text-transform: uppercase;");
        QString avatarPath;
        t.avatarCode == 0 ? avatarPath = "" : avatarPath = t.avatarPath;
        bool alreadySelected = false;
        if( selected.contains(t.id.toString())) alreadySelected = true;
        QReadLocker rl(t.avatar_m.data());
        ClickableLabel *avatar = new ClickableLabel(avatarPath, alreadySelected);
        rl.unlock();
        avatar->setObjectName(t.id.toString());
        avatar->setAlignment(Qt::AlignCenter);
        connect(avatar, &ClickableLabel::clicked, this, &ViewConnectedTibiers::addSelected);
        connect(avatar, &ClickableLabel::unclicked, this, &ViewConnectedTibiers::removeSelected);
        QGridLayout* tibier = new QGridLayout;
        tibier->addWidget(avatar, 0, 0);
        tibier->addWidget(username, 1, 0);
        tibier->setRowMinimumHeight(0, 93);
        if(column == 3) {
            row++;
            column = 0;
        }

        tibiersConnected->addLayout(tibier, row, column, Qt::AlignCenter);
        column++;

    }

}

void ViewConnectedTibiers::removeTibierFromSelected(const QString& id) {
    if( !active ) return;
    if( selected.contains(id) ) {
        selected.removeOne(id);
    }
}


void ViewConnectedTibiers::addSelected(QString id) {
    selected.push_back(id);
}

void ViewConnectedTibiers::removeSelected(QString id) {
    selected.removeOne(id);
}


void ViewConnectedTibiers::createDialog() {
    title.setText("CONNECTED TIBIERS");
    title.setObjectName("title");
    title.setAlignment(Qt::AlignCenter);

    infoText.setAlignment(Qt::AlignCenter);

    sendButton.setText("SEND");
    sendButton.setObjectName("send-button");
    sendButton.setFixedWidth(110);
    connect(&sendButton, &QPushButton::clicked, this, &ViewConnectedTibiers::endSelection);

    cancelButton.setText("CANCEL");
    cancelButton.setObjectName("cancel-button");
    connect(&cancelButton, &QPushButton::clicked, this, &ViewConnectedTibiers::cancelSelectionRequest);

    cancelButton.setFixedWidth(110);
    buttonsLayout.addSpacing(150);
    buttonsLayout.setSpacing(5);
    buttonsLayout.addWidget(&cancelButton);
    buttonsLayout.addWidget(&sendButton);


    verticalLayout.addWidget(&title);
    verticalLayout.addWidget(&infoText);
    verticalLayout.addLayout(&buttonsLayout);
    verticalLayout.setSpacing(13);
    verticalLayout.addSpacing(15);
    setLayout(&verticalLayout);

}

void ViewConnectedTibiers::endSelection() {

    if(selected.isEmpty())  {
        int ret = QMessageBox::question(nullptr, "TIBIERS SELECTION EMPTY", "You haven't selected any tibiers. Do you want to cancel the request?");
        if ( ret == QMessageBox::No ) return;
    }

    hide();
    emit selectionReady(selected);
    selected.clear();
    active = false;

}

void ViewConnectedTibiers::cancelSelectionRequest() {
    hide();
    selected.clear();
    emit selectionReady(selected);
    active = false;
}


void ViewConnectedTibiers::initCentralWidget() {

    row = 0;
    column = 0;

    if(scroll != nullptr) {
        delete scroll;
    }

    centralWidget = new QWidget;
    tibiersConnected = new QGridLayout;
    tibiersConnected->setVerticalSpacing(20);

    centralWidget->setFixedWidth(450);
    centralWidget->setLayout(tibiersConnected);

    column1 = new QLabel;
    column2 = new QLabel;
    column3 = new QLabel;
    column1->setFixedWidth(centralWidget->width()/3);
    column2->setFixedWidth(centralWidget->width()/3);
    column3->setFixedWidth(centralWidget->width()/3);
    tibiersConnected->addWidget(column1, 1, 0);
    tibiersConnected->addWidget(column2, 1, 1);
    tibiersConnected->addWidget(column3, 1, 2);

    scroll = new QScrollArea;
    scroll->setWidget(centralWidget);
    scroll->setWidgetResizable(true);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setStyleSheet("border: 0;");

    verticalLayout.insertWidget(2, scroll);

}





