#ifndef ROW_H
#define ROW_H
#include <QString>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QTime>

class UDRow {
public:
    UDRow(int code, QString tibierName, QString fileName);
    ~UDRow();
    bool abortFlag = false;
    int code;
    QLabel *to = nullptr;
    QLabel *name = nullptr;
    QLabel *time = nullptr;
    QProgressBar *bar = nullptr;
    QPushButton *abort = nullptr;
    QPushButton *open = nullptr;
    QTime lastTime;

};

#endif // ROW_H
