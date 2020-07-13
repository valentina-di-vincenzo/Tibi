/*
 * Copyright (c) 2020 Valentina Di Vincenzo ( hello@valentina-divincenzo.com )
 *  This file is part of Tibi which is released under the GNU General Public License, version 3.0.
 * See file LICENSE or go to http://www.gnu.org/licenses/ for full license details.
 *
 */

#ifndef DOWNLOADMODEL_H
#define DOWNLOADMODEL_H

#include <QNetworkInterface>
#include <QTimer>
#include "download/DownloadHandler.h"
#include "view/ViewConfirmation.h"
#include "user/UserHandler.h"

class DownloadsDispatcher : public QObject
{
    Q_OBJECT
public:
    DownloadsDispatcher() {}
    void setUserInfo(UserHandler* user);
    ~DownloadsDispatcher();


public slots:
    void createDownloadHandler(qintptr socketDescriptor);
    void updateDownload(QSharedPointer<Download> download);

    /**
     * @brief computeAnswer has to dispatch the confirmation request of handlers. If the confirmation mode is disable, it will immediatly return that the answer is accepted, otherwise it opes the confirmation view and ask the user what to do.
     * @param download, the request object
     */
    void computeAnswer(QSharedPointer<Download> download);

    /**
     * @brief newAnswer answer from conf
     * @param download
     * @param status
     */
    void newAnswer(QSharedPointer<Download> download, Status status);

    /**
     * @brief downloadFinished, notify the viewTray to emit a baloon and remove the download from the active ones. It is safe to close the app even if the confirmation window is opened.
     * @param download object request
     */
    void downloadFinished(QSharedPointer<Download> download);

    /**
     * @brief askOverwrite, ask the user if the file/dir has to be overwritten or wants to keep both. The ViewUD is updated with the new name.
     * @param code of the download
     * @param type dir/file
     * @param oldName
     * @param newName
     * @return true if overwrite, false otherwise
     */
    bool askOverwrite(int code, const QString& type, const QString &oldName, const QString &newName);

    /**
     * @brief informCollision inform the user that the download collides with other ongoing ones and that it will be saved with newName. The ViewUD is updated with the new name.
     * @param code
     * @param oldName
     * @param newName
     */
    void informCollision(int code, const QString& oldName, const QString& newName);

    /**
     * @brief abortFromView, the view requested to abort a download. The mediator marks the abort flag of the handler indicated by the download code and emits abortConnection for that specif thread.
     * @param downloadsCode
     */
    void abortFromView(int downloadsCode);

signals:

    /**
     * @brief userAnswer signal emitted to share the user answer with the handler
     * @param code, the hendler unique code
     * @param status, the answer
     */
    void userAnswer(int code, Status status);

    void notifyUserDownloadStatus(QSharedPointer<Download> download);

    void addDownloadToView(bool upload, int code, const QString& tibierName, const QString& fileName);

    void addDownloadToTray();

    void updateRowDownload(bool upload, int code, int perc, const QTime& uploadTime, const QString& infoString, const QString& path = nullptr);

    void updateDownloadName(int code, const QString& newName);

    /**
     * @brief cleanUp, signal to all the current download handler threads
     */
    void cleanUp();

    /**
     * @brief abortDownload, signal emitted when there is just one thread conneced to it. It signals the handler to terminates.
     */
    void abortDownload();


private:
    UserHandler* user = nullptr;
    QMap<int, DownloadHandler*> downloads;
    int downloadsCode = 0;
    ViewConfirmation* conf = nullptr;
    QTimer* timer;

    /**
     * @brief manageConfrimationView the confirmation view close after 2 minutes of inactivity emitting a declined answer.
     * @param download object request
     */
    void manageConfrimationView(QSharedPointer<Download> download);

    /**
     * @brief checkDownloadDirectoryExistance, before creating a new download handler, the existance of the download directory is checked. If it is not valid, it is substitute with the default one.
     */
    void checkDownloadDirectoryExistance();

};




#endif // DOWNLOADMODEL_H
