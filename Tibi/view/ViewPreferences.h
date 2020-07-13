#ifndef VIEWPREFERENCES_H
#define VIEWPREFERENCES_H

#include <QWidget>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QRadioButton>
#include <QCompleter>
#include <QDirModel>
#include <QFileDialog>
#include <QMessageBox>
#include <QReadWriteLock>
#include "view/clickablelabel.h"
#include "user/UserHandler.h"

class ViewPreferences : public QWidget
{
    Q_OBJECT
public:
    ViewPreferences();
    void setUserInfo(UserHandler* user);

public slots:
    void showPreferences();

private:
    UserHandler* user = nullptr;
    QLineEdit* nameLineEdit;
    QLineEdit* downloadLineEdit;
    QRadioButton* online;
    QRadioButton* offline;
    QRadioButton* automatic;
    QRadioButton* ask;
    ClickableLabel* avatar;
    QVBoxLayout* centralLayout;
    QPushButton* saveButton;
    QPushButton* cancelButton;
    void createWidget();
    void selectAvatar();
    void selectDownloadPath();
    void updatePreferences();
    void setStyle();
    qint8 tmpAvatarCode;
    QString tmpAvatarPath = QDir::currentPath();

private slots:
    void savePreferences();
    void restoreDefault();


};

#endif // VIEWPREFERENCES_H
