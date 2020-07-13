#ifndef VIEW_H
#define VIEW_H

#include <QMainWindow>
#include <QDialog>
#include <QMessageBox>
#include <QGridLayout>
#include <QFile>
#include <QApplication>
#include <QPixmap>
#include <QMap>
#include <QProgressBar>
#include <QDebug>
#include <QScrollArea>
#include <QTime>
#include <QTimer>
#include <QUrl>
#include <QScreen>
#include <QDesktopServices>
#include "view/UDRow.h"

class ViewUD : public QMainWindow
{
    Q_OBJECT

public:
    ViewUD();
    ~ViewUD();

public slots:
    void addUD(bool upload, int code, const QString& tibierName, const QString& fileName);
    void updateUD(bool upload, int code, int perc, const QTime& uploadTime, const QString& infoString, const QString& path = nullptr);
    void raiseView();
    void updateDownloadName(int code, const QString& newName);

signals:
    void signalUploadAbort(int code);
    void signalDownloadAbort(int code);

private:
    QGridLayout* uploadsLayout;
    QGridLayout* downloadsLayout;
    QMap<int, UDRow*>* uploadRows;
    QMap<int, UDRow*>* downloadRows;
    void createSendingWindow();
    void removeItem(QMap<int, UDRow*>* map, QGridLayout* layout, UDRow *row);
    void completed( QMap<int, UDRow*>* map, QGridLayout* layout, UDRow* row);
    void abortingState(int code);

    QLabel* createLogo();
    QLabel* createTitle(const QString& title);
    QHBoxLayout* createTitleLayout(const QString& title);
    QHBoxLayout* createInfoLayout(const QString& toOrFrom);
    QLabel* createSeparator();
    QScrollArea* createScrollArea(QGridLayout* layout);
    QVBoxLayout* createCentralLayout(const QString& title, const QString& toOrFrom, QGridLayout* layout);

};

#endif // VIEW_H
