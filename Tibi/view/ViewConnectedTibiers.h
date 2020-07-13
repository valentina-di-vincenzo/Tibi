#ifndef VIEWSELECTOR_H
#define VIEWSELECTOR_H

#include "clickablelabel.h"
#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QCloseEvent>
#include <QReadWriteLock>
#include <QMessageBox>
#include "user/UserHandler.h"

class ViewConnectedTibiers : public QWidget
{
    Q_OBJECT

public:
    ViewConnectedTibiers();
    void setUserInfo(UserHandler* user);

signals:
    void selectionReady(const QVector<QString>& selected);

public slots:
    void update();
    void showConnectedTibiers(const QVector<QString>& filePaths);
    void removeTibierFromSelected(const QString& id);

private:
    /* -- UTILITIES --*/
    UserHandler* user = nullptr;
    bool active = false;
    QVector<QString> selected;
    QVector<QString> filePaths;
    QVBoxLayout verticalLayout;
    QGridLayout* tibiersConnected;
    int column = 0;
    int row = 0;
    QWidget* centralWidget = nullptr;
    QLabel *column1;
    QLabel *column2;
    QLabel *column3;
    QScrollArea* scroll = nullptr;
    QHBoxLayout buttonsLayout;
    QPushButton sendButton;
    QPushButton cancelButton;
    QLabel title;
    QLabel infoText;
    void createDialog();
    void initCentralWidget();

private slots:
    void addSelected(QString id);
    void removeSelected(QString id);
    void endSelection();
    void cancelSelectionRequest();

protected:
    void closeEvent(QCloseEvent *event) override;


};

#endif // VIEWSELECTOR_H
