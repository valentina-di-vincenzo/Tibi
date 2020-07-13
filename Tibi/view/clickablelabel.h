#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QLabel>
#include <QWidget>
#include <Qt>
#include <QDebug>
#include <QBitmap>
#include <QPixmap>
#include <QPainter>

/**
 * @brief The ClickableLabel class is used to produce a clickable rounded label given the img path. When clicked it emits the object name.
 */

class ClickableLabel : public QLabel {
    Q_OBJECT

public:
    explicit ClickableLabel(QString avatarPath = "", bool selected = false, bool alwaysSelected = false);
    ~ClickableLabel();
    void setAvatar(QString avatarPath = "");

signals:
    void clicked(const QString& id);
    void unclicked(const QString& id);

protected:
    void mousePressEvent(QMouseEvent* event);
    void paintEvent(QPaintEvent* event);

private:
    bool selected = false;
    bool alwaysSelected = false;
    QString avatarPath;
    QBrush* brush = nullptr;
    QLinearGradient* linearGrad = nullptr;

};

#endif // CLICKABLELABEL_H
