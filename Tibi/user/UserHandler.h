/*
 * Copyright (c) 2020 Valentina Di Vincenzo ( hello@valentina-divincenzo.com )
 *  This file is part of Tibi which is released under the GNU General Public License, version 3.0.
 * See file LICENSE or go to http://www.gnu.org/licenses/ for full license details.
 *
 */

#ifndef TIBIMODEL_H
#define TIBIMODEL_H


#include <QVector>
#include <QtNetwork>
#include <QMutex>
#include <QReadLocker>
#include <QReadWriteLock>
#include <QMutexLocker>
#include <QTimer>
#include <QSharedPointer>
#include <QStandardPaths>
#include "user/tibier.h"

#define DISCONNECTED_CHECK_TIME 10*1000
#define MAX_TIME_BETWEEN_PING 12


class UserHandler : public QObject
{
    Q_OBJECT

public:
    static UserHandler* Instance();

    ~UserHandler();
    void createUser();

    /**
     * @brief Thread-safe set and get username
     * @return const ref to username
     */
    QString getUsername();
    void setUsername(const QString& newUsername);

    /**
     * @brief Thread-safe getter for avatarcode
     */
    qint8 getAvatarCode();
    const QString& getAvatarPath();

    /**
      * @brief saves the new avatar, called "test", changed in preferences and updates the avatar sent in ping
    */
    void updateUserAvatar(qint8 avatarCode);

    /**
     * @brief Thread-safe set and get status
     * @return bool - status true = online
     */
    bool getStatus();
    void setStatus(bool newState);

    /**
     * @brief Thread-safe set and get confirmation mode
     * @return bool - mode true = confirmation enabled
     */
    bool getConfirmation();
    void setConfirmation(bool newMode);

    /**
     * @brief Thread-safe set and get downloadPath
     * @return const ref to download path
     */
    const QString& getDownloadPath();
    void setDownloadPath(const QString& newPath);

    const QHostAddress& getAddress();
    int getPort();
    const QUuid& getId();

    void restoreDefaultSettings();

    /**
     * @brief getLockFromId used to print the avatar in the confirmation view ( that doesn't have the userHandler istance )
     * @param id to retrieve the lock
     * @return the lock
     */
    QSharedPointer<QReadWriteLock> getLockFromId(const QUuid& id);

    /**
      * @brief return a vector of connected tibier, used by connectedView
      * @return QSharedPointer<QVector<Tibier>> - a shared pointer to the vector of online tibiers
    */
    QSharedPointer<QVector<Tibier>> readCurrentlyConnectedTibiers();

    /**
      * @brief given a vector of ids, returns the corresponding tibiers, if online. Used by UploadDispatcher.
      * @param const QVector<QString>& - the vector of selected string id
      * @return QSharedPointer<QVector<Tibier>> - a shared pointer to the vector of tibiers
    */
    QSharedPointer<QVector<Tibier>> getTibiersFromId(const QVector<QString>& selectedId);

    /**
     * @brief checkIfNewAvatarTibier, check if the avatar code is different from the one stored or if it is new
     * @param id
     * @param avatarCode
     * @return true if TibiDiscovery has to save the avatar, false otherwise
     */
    bool checkIfNewAvatarTibier(const QUuid& id, int avatarCode);

    /**
     * @brief newPing, analyse a new ping, probabily updating the field of the tibier
     * @param tibier, the object where the new info are stored
     * @param avatarArrayPing, the avatarArray downloaded and to be saved, if necessary
     */
    void newPing(QSharedPointer<Tibier> tibier, QSharedPointer<QByteArray> avatarArrayPing = nullptr);

    /**
     * @brief saveNewTibierAvatar thread-safe way to save the new avatar ping related to a connected tibier
     * @param avatarPath
     * @param avatarArrayPing
     */
    void saveNewTibierAvatar(const QString& avatarPath, QSharedPointer<QByteArray> avatarArrayPing);


public slots:
    void setReceiverAddress(QHostAddress address, int port);

signals:
    void userReady();
    void updateConnectedTibiers();
    void removeTibierFromSelected(QString id);
    bool updateAvatarPing(qint8 avatarCode);
    void updateStatus(bool online);

private:
    UserHandler() {}
    Tibier user;
    /* -- user preferences --*/
    QString downloadPath;
    bool confirmation;
    QReadWriteLock online_m;
    QReadWriteLock username_m;
    QReadWriteLock downloadPath_m;
    QReadWriteLock confirmation_m;

    /* -- tibier connected --*/
    QTimer disconnectedTimer;
    QReadWriteLock connected_m;
    QMap<QUuid, QSharedPointer<Tibier>> tibiers;

    /* -- new ping -- */
    QSharedPointer<Tibier> newTibierPing;
    QSharedPointer<Tibier> tibierToUpdate;
    QSharedPointer<QByteArray> avatarArrayPing;
    bool newTibier();
    void saveNewTibier();
    void updateOldTibier();
    bool newUsername();
    bool newAvatar();
    bool newState();


    /**
     * @brief restoreSettings, restores the setting and creates the avatar direcotry using QStandsPaths::AppDataLocation
     */
    void restoreSettings();
    void firstSettings(QSettings&& settings);
    void fillSettings(QSettings&& settings);

    void saveSettings();

private slots:
    void checkDisconnected();

signals:
    void startToCheckDisconnected();

};





#endif // TIBIMODEL_H
