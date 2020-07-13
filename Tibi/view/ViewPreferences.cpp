/*
 * Copyright (c) 2020 Valentina Di Vincenzo ( hello@valentina-divincenzo.com )
 *  This file is part of Tibi which is released under the GNU General Public License, version 3.0.
 * See file LICENSE or go to http://www.gnu.org/licenses/ for full license details.
 *
 */

#include "ViewPreferences.h"

void ViewPreferences::setUserInfo(UserHandler* user) {
    this->user = user;
}

ViewPreferences::ViewPreferences() : QWidget()
{

    setObjectName("preferences");
    createWidget();
    setFixedHeight(600);
    hide();
}



void ViewPreferences::showPreferences() {
    updatePreferences();
    show();
    raise();

}

void ViewPreferences::updatePreferences() {
    QString username = user->getUsername();
    nameLineEdit->setText(username);
    downloadLineEdit->setText(user->getDownloadPath());
    user->getStatus() ? online->setChecked(true) : offline->setChecked(true);
    user->getConfirmation() ? ask->setChecked(true) : automatic->setChecked(true);
    avatar->setAvatar(user->getAvatarCode() == 0? "" : user->getAvatarPath() + ".jpg");
    tmpAvatarCode = user->getAvatarCode();
    cancelButton->setVisible(true);

}

void ViewPreferences::createWidget() {

    /* -- TITLE --*/
    QLabel* title = new QLabel("PREFERENCES");
    title->setObjectName("title");

    /* -- AVATAR -- */
    avatar = new ClickableLabel("", true, true);
    avatar->setFixedHeight(90);

    /* -- USERNAME --*/
    nameLineEdit = new QLineEdit;
    nameLineEdit->setFixedHeight(30);
    nameLineEdit->setObjectName("username-lineedit");
    nameLineEdit->setAttribute(Qt::WA_MacShowFocusRect, 0);
    nameLineEdit->setInputMask("nnnnnnnnnnnnnnnnnnnnnnn");


    /* -- EDIT AVATAR BUTTON --*/
    QPushButton* avatarButton = new QPushButton("CHANGE AVATAR");
    avatarButton->setObjectName("select-button");
    connect(avatarButton, &QPushButton::clicked, this, &ViewPreferences::selectAvatar);

    /* -- FORM -- */

    QPushButton* downloadButton = new QPushButton("select");
    downloadButton->setObjectName("select-button");
    connect(downloadButton, &QPushButton::clicked, this, &ViewPreferences::selectDownloadPath);
    QLabel* downloadLabel = new QLabel("Download path: ");
    downloadLabel->setObjectName("preferences-label");
    downloadLineEdit = new QLineEdit;
    downloadLineEdit->setFixedHeight(30);
    downloadLineEdit->setAttribute(Qt::WA_MacShowFocusRect, 0);
    downloadLineEdit->setReadOnly(true);


    QGridLayout* formLayout = new QGridLayout;

    formLayout->addWidget(downloadLabel, 1, 0);
    formLayout->addWidget(downloadLineEdit, 2, 0);
    formLayout->addWidget(downloadButton, 2, 1);

    formLayout->setSpacing(5);
    formLayout->setHorizontalSpacing(10);



    /* -- RADIO BUTTONS -- */
    QGroupBox *statusBox = new QGroupBox("Status");
    online = new QRadioButton("  online");
    offline = new QRadioButton("  offline");

    QVBoxLayout *statusLayout = new QVBoxLayout;
    statusLayout->addWidget(online);
    statusLayout->addWidget(offline);
    statusLayout->setSpacing(20);
    statusBox->setFixedWidth(100);
    statusBox->setLayout(statusLayout);

    QGroupBox *confirmBox = new QGroupBox("Before downloading..");
    ask = new QRadioButton("  always ask");
    automatic = new QRadioButton("  accept all");

    QVBoxLayout *confirmLayout = new QVBoxLayout;
    confirmLayout->addWidget(ask);
    confirmLayout->addWidget(automatic);
    confirmLayout->setSpacing(20);
    confirmBox->setLayout(confirmLayout);

    QHBoxLayout* radioLayout = new QHBoxLayout;
    radioLayout->addWidget(statusBox);
    radioLayout->addWidget(confirmBox);

    /* -- BUTTONS --*/
    saveButton = new QPushButton("SAVE");
    saveButton->setObjectName("send-button");
    saveButton->setFixedWidth(110);
    connect(saveButton, &QPushButton::clicked, this, &ViewPreferences::savePreferences);

    cancelButton = new QPushButton("CANCEL");
    cancelButton->setObjectName("cancel-button");
    connect(cancelButton, &QPushButton::clicked, this, &ViewPreferences::close);
    cancelButton->setFixedWidth(110);

    QPushButton* defaultButton = new QPushButton("DEFAULT");
    defaultButton->setObjectName("cancel-button");
    connect(defaultButton, &QPushButton::clicked, this, &ViewPreferences::restoreDefault);
    defaultButton->setFixedWidth(110);

    QHBoxLayout* buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(defaultButton);
    buttonsLayout->addSpacing(30);
    buttonsLayout->setSpacing(10);
    buttonsLayout->addWidget(cancelButton);
    buttonsLayout->addWidget(saveButton);

    /* -- DEPLOY -- */
    centralLayout = new QVBoxLayout;
    centralLayout->addWidget(title);
    centralLayout->setAlignment(title, Qt::AlignCenter);
    centralLayout->addWidget(avatar);
    centralLayout->setAlignment(avatar, Qt::AlignCenter);
    centralLayout->addWidget(nameLineEdit);
    centralLayout->setAlignment(nameLineEdit, Qt::AlignCenter);
    centralLayout->addWidget(avatarButton);
    centralLayout->setAlignment(avatarButton, Qt::AlignCenter);
    centralLayout->addLayout(formLayout);
    centralLayout->addLayout(radioLayout);
    centralLayout->addLayout(buttonsLayout);
    centralLayout->addSpacing(10);
    centralLayout->setSpacing(20);
    setLayout(centralLayout);

    setContentsMargins(15, 0, 15, 0);
}



void ViewPreferences::selectAvatar() {
    qDebug() << "request to change avatar";
    QString path = QFileDialog::getOpenFileName(
                this,
                "Select new avatar",
                tmpAvatarPath,
                "JPG files (*.jpg)");
    if(path == "") return;
    tmpAvatarPath = path;
    QImage img(tmpAvatarPath);
    QPixmap pixmap;
    pixmap = pixmap.fromImage(img.scaled(80,80,Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation));

    QFile newAvatarFile(user->getAvatarPath() + "-test.jpg");
    newAvatarFile.open(QIODevice::WriteOnly);
    pixmap.save(&newAvatarFile, "jpg",100);

    if( newAvatarFile.size() > 8000 ) { // 8kB
        qDebug() << "immagine troppo grossa " << newAvatarFile.size();
        QMessageBox::critical(nullptr, QObject::tr("Avatar"),
                              QObject::tr("The image is too big (max 8kB). Select a smaller one, please."));
        QFile avatarTest(user->getAvatarPath() + "-test.jpg");
        avatarTest.remove();
    }
    else {
        tmpAvatarCode = user->getAvatarCode()+1;
        avatar->setAvatar(user->getAvatarPath() + "-test.jpg");

    }
    newAvatarFile.close();

}

void ViewPreferences::selectDownloadPath() {
    QString tmpDownloadPath =  QFileDialog::getExistingDirectory(this, tr("Select Download Directory"),
                                                              user->getDownloadPath(),
                                                              QFileDialog::ShowDirsOnly
                                                              | QFileDialog::DontResolveSymlinks);
    if(tmpDownloadPath != "") {
        downloadLineEdit->setText(tmpDownloadPath);
    }
    downloadLineEdit->setFocus();
}

void ViewPreferences::restoreDefault() {
    int ret = QMessageBox::question(this, "RESTORE DEFAULT SETTINGS", "The operation will be irreversible. Do you want to continue?");
    if( ret == QMessageBox::No ) return;
    user->restoreDefaultSettings();
    updatePreferences();
    cancelButton->setVisible(false);


}

void ViewPreferences::savePreferences() {
    if( nameLineEdit->hasAcceptableInput() ) {
        user->setUsername(nameLineEdit->text().toUpper());
    }
    online->isChecked() ? user->setStatus(true) : user->setStatus(false);
    ask->isChecked() ? user->setConfirmation(true) : user->setConfirmation(false);
    user->setDownloadPath(downloadLineEdit->text());

    if( tmpAvatarCode != user->getAvatarCode() ) {
        user->updateUserAvatar(tmpAvatarCode);
    }

    close();

    qDebug() << "new preferences: "
             << "username: " << user->getUsername()
             << "downloadPath: " << user->getDownloadPath()
             << "avatarCode " << user->getAvatarCode()
             << "online: " << user->getStatus()
             << "confirmation? " << user->getConfirmation();
}

