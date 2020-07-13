/*
 * Copyright (c) 2020 Valentina Di Vincenzo ( hello@valentina-divincenzo.com )
 *  This file is part of Tibi which is released under the GNU General Public License, version 3.0.
 * See file LICENSE or go to http://www.gnu.org/licenses/ for full license details.
 *
 */

#include "UserHandler.h"

UserHandler* UserHandler::Instance()
{
    static UserHandler instance;
    return &instance;
}

void UserHandler::setReceiverAddress(QHostAddress userAddress, int userPort) {
    user.address = userAddress;
    user.port = userPort;
    emit userReady();

}


void UserHandler::createUser() {
    user.avatarPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/user-avatar";
    restoreSettings();
    //.jpg
    connect(&disconnectedTimer, &QTimer::timeout, this, &UserHandler::checkDisconnected);
    connect(this, &UserHandler::startToCheckDisconnected, this, &UserHandler::checkDisconnected, Qt::QueuedConnection);
}


bool UserHandler::checkIfNewAvatarTibier(const QUuid& id, int newAvatarCode) {
    QReadLocker rl(&connected_m);
    if( !tibiers.contains(id) ) return true; //new tibier
    if( tibiers.value(id)->avatarCode != newAvatarCode) return true; //old tibier but new avatar
    return false;
}


void UserHandler::checkDisconnected() {
    if( !disconnectedTimer.isActive() ) disconnectedTimer.start(DISCONNECTED_CHECK_TIME);
    QWriteLocker ml(&connected_m);
    bool somethingChanged = false;
    bool tibiersOnline = false;
    //qDebug() << "Registred tibier: " << tibiers.size();
    for( auto t : tibiers ) {
        //qDebug() << "tibier online? " << t->online;
        if ( !t->online ) continue;
        qint64 diff = QDateTime::currentSecsSinceEpoch() - t->lastPing;
        //qDebug() << "user handler: diff between last ping " << diff;
        if( diff > MAX_TIME_BETWEEN_PING) {
            somethingChanged = true;
            t->online = false;
            //qDebug() << "user handler: removing tibier " << t->username;
            emit removeTibierFromSelected(t->id.toString());
        } else {
            tibiersOnline = true;
        }
    }

    if( somethingChanged ) {
        emit updateConnectedTibiers();
    }
    if( !tibiersOnline ) disconnectedTimer.stop();
}



const QString& UserHandler::getAvatarPath() {
    return user.avatarPath;
}

const QHostAddress& UserHandler::getAddress() {
    return user.address;
}

int UserHandler::getPort() {
    return user.port;
}

const QUuid& UserHandler::getId() {
    return user.id;
}


const QString& UserHandler::getDownloadPath() {
    QReadLocker rl(&downloadPath_m);
    return downloadPath;
}

void UserHandler::setDownloadPath(const QString &newPath) {
    QWriteLocker rl(&downloadPath_m);
    downloadPath = newPath;
}


QString UserHandler::getUsername() {
    QReadLocker rl(&username_m);
    return user.username;
}

void UserHandler::setUsername(const QString &newUsername) {
    QWriteLocker wl(&username_m);
    user.username = newUsername;
}


bool UserHandler::getStatus() {
    QReadLocker rl(&online_m);
    return user.online;
}

void UserHandler::setStatus(bool newState) {
    QWriteLocker wl(&online_m);
    user.online = newState;
    emit updateStatus(newState);
}



bool UserHandler::getConfirmation() {
    QReadLocker rl(&confirmation_m);
    return confirmation;
}

void UserHandler::setConfirmation(bool newMode) {
    QWriteLocker rl(&confirmation_m);
    confirmation = newMode;
}


qint8 UserHandler::getAvatarCode() {
    QReadLocker rl(user.avatar_m.data());
    return user.avatarCode;
}


void UserHandler::updateUserAvatar(qint8 avatarCode) {
    QWriteLocker wl(user.avatar_m.data());
    user.avatarCode = avatarCode;
    if( user.avatarCode == 0 ) {
        emit updateAvatarPing(avatarCode);
        return;
    }//default avatar
    QFile oldAvatar(user.avatarPath + ".jpg");
    oldAvatar.remove();
    QFile newAvatarFile(user.avatarPath + "-test.jpg");
    newAvatarFile.rename(user.avatarPath + ".jpg");
    bool updated = emit updateAvatarPing(avatarCode);
    if ( !updated ) user.avatarCode = 0;
    qDebug() << "user.avatarcode = " << user.avatarCode;
}

QSharedPointer<QReadWriteLock> UserHandler::getLockFromId(const QUuid& id) {
    QWriteLocker wl(&connected_m);
    if( tibiers.contains(id)) {
        return tibiers.value(id)->avatar_m;
    }
    else return nullptr;
}


QSharedPointer<QVector<Tibier>> UserHandler::readCurrentlyConnectedTibiers() {
    qDebug() << "read currently connected tibiers called";
    QReadLocker rl(&connected_m);
    QSharedPointer<QVector<Tibier>> connectedTibier = QSharedPointer<QVector<Tibier>>(new QVector<Tibier>);
    for( auto t : tibiers ) {
        qDebug() << "handler get tibier: " << t->username
                 << "\tonline? " << t->online;
        if( t->online )
            connectedTibier->push_back(*t);
    }
    return connectedTibier;
}



QSharedPointer<QVector<Tibier>> UserHandler::getTibiersFromId(const QVector<QString>& selectedId) {
    QReadLocker rl(&connected_m);
    QSharedPointer<QVector<Tibier>> selectedTibiers = QSharedPointer<QVector<Tibier>>(new QVector<Tibier>);
    for( auto idString : selectedId ) {
        qDebug() << "handler get tibier - id: " << idString;
        QUuid id(idString);
        if( !tibiers.contains(id) ) continue;
        Tibier selected = *tibiers.value(id);
        if ( !selected.online ) continue;
        selectedTibiers->push_back(selected);
        qDebug() << "handler get tibier - username: " << selected.username;

    }
    return selectedTibiers;
}


void UserHandler::newPing(QSharedPointer<Tibier> tibierPing, QSharedPointer<QByteArray> avatarArrayPing) {

    if( !disconnectedTimer.isActive() ) emit startToCheckDisconnected();
    QWriteLocker wl(&connected_m);

    newTibierPing = tibierPing;
    this->avatarArrayPing = avatarArrayPing;
    if( !tibiers.contains(newTibierPing->id) ) {
        saveNewTibier();
        return;
    }

    updateOldTibier();
}


void UserHandler::saveNewTibier() {
    qDebug() << QThread::currentThreadId() << " - USER HANDLER: New Tibier Connected" << newTibierPing->username;
    tibiers.insert(newTibierPing->id, newTibierPing);
    saveNewTibierAvatar(newTibierPing->avatarPath, avatarArrayPing);
    emit updateConnectedTibiers();
}

void UserHandler::updateOldTibier() {
    tibierToUpdate = tibiers.value(newTibierPing->id);
    tibierToUpdate->lastPing = newTibierPing->lastPing;
    if( newUsername() || newAvatar() || newState() ) {
        emit updateConnectedTibiers();
    }

}

bool UserHandler::newUsername() {
    if( tibierToUpdate->username != newTibierPing->username ) {
        tibierToUpdate->username = newTibierPing->username;
        return true;
    }

    return false;
}

bool UserHandler::newAvatar() {
    if( tibierToUpdate->avatarCode != newTibierPing->avatarCode) {
        tibierToUpdate->avatarCode = newTibierPing->avatarCode;
        if( tibierToUpdate->avatarCode != 0 ) {
            QWriteLocker wl(tibierToUpdate->avatar_m.data());
            saveNewTibierAvatar(tibierToUpdate->avatarPath, avatarArrayPing);
        }
        return true;
    }
    return false;
}

bool UserHandler::newState() {
    if( tibierToUpdate->online != newTibierPing->online ) {
        tibierToUpdate->online = newTibierPing->online;
        return true;
    }
    return false;
}

void UserHandler::saveNewTibierAvatar(const QString& avatarPath, QSharedPointer<QByteArray> avatarArrayPing) {
    QFile avatarFile(avatarPath);
    if(avatarFile.open(QIODevice::WriteOnly)) {

        qint64 written = 0;
        while( written < avatarArrayPing->size() ) {
            written += avatarFile.write(*avatarArrayPing);
        }
    }

}


void UserHandler::restoreSettings() {
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    dir.mkpath("avatars");

    qDebug() << QThread::currentThreadId() << " - TIBI MODEL restore setting";

    QSettings settings("valentina-di-vincenzo, Tibi");
    QString firstSetting = settings.value("firstSetting", "yes").toString();
    if( firstSetting  == "yes" ) {
        firstSettings(std::move(settings));
    }
    fillSettings(std::move(settings));
    user.id = QUuid::createUuid();

    qDebug() << user.username
             << downloadPath
             << "\nonline?" << user.online
             << "\nConfirmation?" << confirmation
             << user.id.toString() << user.avatarCode
             << "\nuser avatar path: " << user.avatarPath << ".jpg";

}

void UserHandler::firstSettings(QSettings&& settings) {
    settings.setValue("firstSetting", "no");
    QString username = qgetenv("USER");
    if (username.isEmpty()) {
        username = qgetenv("USERNAME").toUpper();
    }
    settings.setValue("name", username);
    QString downloadPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    settings.setValue("download", downloadPath);
    settings.setValue("online", 0);
    settings.setValue("confirmation", 1);
    settings.setValue("avatarCode", 0);
}


void UserHandler::fillSettings(QSettings&& settings) {
    setUsername(settings.value("name").toString());
    setDownloadPath(settings.value("download").toString());
    setStatus(settings.value("online").toInt());
    setConfirmation(settings.value("confirmation").toInt());
    QWriteLocker wl(user.avatar_m.data());
    user.avatarCode = settings.value("avatarCode").toInt();
    emit updateAvatarPing(user.avatarCode);
}

void UserHandler::saveSettings() {

    QDir avatarsDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/avatars");
    avatarsDir.removeRecursively();

    QSettings settings("valentina-di-vincenzo, Tibi");
    settings.setValue("name", user.username);
    settings.setValue("download", downloadPath);
    settings.setValue("online", user.online);
    settings.setValue("confirmation", confirmation);
    if( user.avatarCode != 0 ) user.avatarCode = 1;
    settings.setValue("avatarCode", user.avatarCode);

}

void UserHandler::restoreDefaultSettings() {
    QSettings settings("valentina-di-vincenzo, Tibi");
    firstSettings(std::move(settings));
    fillSettings(std::move(settings));
}

UserHandler::~UserHandler() {
    saveSettings();
}





