/*
 * Copyright (c) 2020 Valentina Di Vincenzo ( hello@valentina-divincenzo.com )
 *  This file is part of Tibi which is released under the GNU General Public License, version 3.0.
 * See file LICENSE or go to http://www.gnu.org/licenses/ for full license details.
 *
 */

#include "ViewTray.h"

ViewTray::ViewTray() {
    setStyle();
    trayIconMenu = new QMenu;

}

void ViewTray::setUserInfo(UserHandler *user) {
    this->user = user;
    createActions();
    setContextMenu(trayIconMenu);
    show();
}

void ViewTray::setStyle() {
    QFile file(":/style/style/style.qss");
    if(file.open(QFile::ReadOnly)) {
        QString stylesheet = QLatin1String(file.readAll());
        qApp->setStyleSheet(stylesheet);
    }
}

void ViewTray::createActions() {

    createSwitchModeAction();
    trayIconMenu->addSeparator();
    createUploadCounter();
    createDownloadCounter();
    trayIconMenu->addSeparator();
    createMaximiseUploadsAction();
    createPreferencesAction();
    trayIconMenu->addSeparator();
    createQuitAction();

}

void ViewTray::createUploadCounter() {
    uploads = trayIconMenu->addAction("Uploads: " + QString::number(uploadsCounter));
}

void ViewTray::createDownloadCounter() {
    downloads = trayIconMenu->addAction("Downloads: " + QString::number(downloadsCounter));
}

void ViewTray::createPreferencesAction() {
    QAction* preferencesAction = trayIconMenu->addAction("Preferences");
    connect(preferencesAction, &QAction::triggered, this, &ViewTray::openPreferences);
}

void ViewTray::createSwitchModeAction() {
    modeAction = trayIconMenu->addAction("");
    updateStatusAction(user->getStatus());
    connect(modeAction, &QAction::triggered, this, &ViewTray::switchStatus);
}

void ViewTray::switchStatus() {
    user->setStatus(!user->getStatus());
}

void ViewTray::updateStatusAction(bool newState) {
    if( newState ) {
        modeAction->setText("Go offline");
        setIcon(QIcon(":/images/images/tibi-logo-tray-online.png"));
    } else {
        modeAction->setText("Go online");
        setIcon(QIcon(":/images/images/tibi-logo-tray-offline.png"));
    }
}
void ViewTray::createQuitAction() {
    trayIconMenu->addAction("Quit Tibi", this, &ViewTray::askBeforeQuitting);

}

void ViewTray::createMaximiseUploadsAction() {
    QAction* maximise = trayIconMenu->addAction("Show transfers");
    connect(maximise, &QAction::triggered, this, &ViewTray::raiseUploads);

}

void ViewTray::askBeforeQuitting() {
    if( downloadsCounter == 0 && uploadsCounter == 0 ) {
        emit closeApplication();
        return;
    }

    QMessageBox msgBox;
    msgBox.setText("There are some pending transfers.");
    msgBox.setInformativeText("Do you want to quit and abort all the ongoing trasnfers?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();

    if( ret == QMessageBox::Yes ) {
        emit closeApplication();
    }

}


void ViewTray::notifyUserDownloadStatus(QSharedPointer<Download> download) {

    QPixmap pixmapLogo(":/images/images/tibi-logo.png");
    pixmapLogo = pixmapLogo.scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QIcon icon(pixmapLogo);
    QString msg = "Download of " + download->itemName;
    if( download->status != Status::WaitingForAnswer ) {
        removeDownload();
        if( download->status == Status::AbortedFromUser)
            return;
    } else {
        msg += " started.";
    }
    if( download->status == Status::Completed ) {
        msg += " completed.";
    }
    else if( download->status == Status::Error || download->status == Status::Failed ) {
        msg += " failed.";
    }
    else if( download->status == Status::Aborted ) {
        msg += " aborted by tibier " + download->usernameSender;
    }

    showMessage("TIBI NOTIFICATION", msg, icon);
}

void ViewTray::addUpload() {
    uploadsCounter++;
    uploads->setText("Uploads: " + QString::number(uploadsCounter));
}

void ViewTray::removeUpload() {
    if( uploadsCounter != 0) uploadsCounter--;
    uploads->setText("Uploads: " + QString::number(uploadsCounter));
}

void ViewTray::addDownload() {
    downloadsCounter++;
    downloads->setText("Downloads: " + QString::number(downloadsCounter));
}

void ViewTray::removeDownload() {
    if( downloadsCounter != 0 ) downloadsCounter--;
    downloads->setText("Downloads: " + QString::number(downloadsCounter));
}

ViewTray::~ViewTray() {
    delete trayIconMenu;
}


