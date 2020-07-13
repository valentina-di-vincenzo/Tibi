#ifndef VIEWCONFIRMATION_H
#define VIEWCONFIRMATION_H
#include "download/Download.h"
#include "view/clickablelabel.h"
#include <QWidget>
#include <QMainWindow>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QCloseEvent>
#include <QStandardPaths>

class ViewConfirmation : public QWidget
{
    Q_OBJECT

public:

    ViewConfirmation(const Download& download);
    void showConfirmation(QSharedPointer<QReadWriteLock> avatar_m);
    int code = -1;

signals:

    void userAnswer(bool accepted);

private:
   const Download download;
   QString computeSize();

    void createMessage();

protected:
    void closeEvent(QCloseEvent *event) override;

};

#endif // VIEWCONFIRMATION_H
