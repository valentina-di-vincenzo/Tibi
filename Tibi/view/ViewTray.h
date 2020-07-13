#ifndef VIEWTRAY_H
#define VIEWTRAY_H

#include <QSystemTrayIcon>
#include <QMainWindow>
#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QApplication>
#include "download/Download.h"
#include "user/UserHandler.h"

class ViewTray : public QSystemTrayIcon
{
    Q_OBJECT
public:
    ViewTray();
    void setUserInfo(UserHandler* user);
    ~ViewTray();

signals:
    void openPreferences();
    void raiseUploads();
    void closeApplication();

public slots:
    void addDownload();
    void addUpload();
    void removeUpload();
    void notifyUserDownloadStatus(QSharedPointer<Download> download);
    void updateStatusAction(bool newState);


private:

    UserHandler* user = nullptr;
    int downloadsCounter = 0;
    int uploadsCounter = 0;

    /* -- ACTIONS --*/
    QAction* modeAction;
    QAction* uploads;
    QAction* downloads;
    QMenu *trayIconMenu = nullptr;
    void createActions();
    void createPreferencesAction();
    void createSwitchModeAction();
    void createQuitAction();
    void createMaximiseUploadsAction();
    void createUploadCounter();
    void createDownloadCounter();
    void setStyle();
    void removeDownload();

private slots:
    void askBeforeQuitting();
    void switchStatus();
};

#endif // VIEWTRAY_H
