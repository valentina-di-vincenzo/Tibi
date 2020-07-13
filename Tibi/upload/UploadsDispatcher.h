/*
 * Copyright (c) 2020 Valentina Di Vincenzo ( hello@valentina-divincenzo.com )
 *  This file is part of Tibi which is released under the GNU General Public License, version 3.0.
 * See file LICENSE or go to http://www.gnu.org/licenses/ for full license details.
 *
 */

#ifndef UPLOADSMEDIATOR_H
#define UPLOADSMEDIATOR_H

#include "UploadHandler.h"

class UploadsDispatcher : public QObject
{
    Q_OBJECT
public:
    UploadsDispatcher() {}
    void setUserInfo(UserHandler* user);
    ~UploadsDispatcher();

public slots:
    void newFileSelection(const QVector<QString>& filePaths);
    void tibiersSelectionReady(const QVector<QString>& selectedId);

    void updateUpload(QSharedPointer<Upload> upload);
    void abortFromView(int uploadCode);

signals:
    void showConnectedTibiers(const QVector<QString>& filePaths);
    void endSelectionSession();
    void addUploadToView(bool upload, int code, const QString& tibierName, const QString& fileName);
    void addUploadToTray();
    void removeUploadFromTray();
    void updateRowUpload(bool upload, int code, int perc, const QTime& uploadTime, const QString& infoString, const QString& path = nullptr);
    void cleanUp();
    void abortUpload();

   private:
    UserHandler* user = nullptr;
    int uploadCode = 0;
    QMap<int, UploadHandler*> uploads;
    QMap<QString, QString> selectedFilePathsAndNames;
    void createUploadHandler(int code, Tibier& tibier, const QString& filePath);
    void connectUploadHandler(UploadHandler* handler);
    void clearSelectionSession();
    void computeFileNames(QVector<QString>& fileNames);

};

#endif // UPLOADSMEDIATOR_H
