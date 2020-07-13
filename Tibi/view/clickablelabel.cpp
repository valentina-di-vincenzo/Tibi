/*
 * Copyright (c) 2020 Valentina Di Vincenzo ( hello@valentina-divincenzo.com )
 *  This file is part of Tibi which is released under the GNU General Public License, version 3.0.
 * See file LICENSE or go to http://www.gnu.org/licenses/ for full license details.
 *
 */

#include "clickablelabel.h"

ClickableLabel::ClickableLabel( QString avatarPath, bool selected, bool alwaysSelected)
    :
      selected(selected),
      alwaysSelected(alwaysSelected)

{
    setFixedWidth(93);

    linearGrad = new QLinearGradient(QPointF(0, 0), QPointF(70, 70));
    setAvatar(avatarPath);


}

void ClickableLabel::setAvatar(QString avatarPath) {
    qDebug() << "Clickable label avatar path received: " << avatarPath;
    this->avatarPath = avatarPath;
    QPixmap pixmap(avatarPath);
    delete brush;
    if( pixmap.isNull() ) {
        linearGrad->setColorAt(0, "#00FDFF");
        linearGrad->setColorAt(1, "#FF2F92");
        brush = new QBrush(*linearGrad);
    }
    else {

        brush = new QBrush(pixmap);
    }

    repaint();

}

void ClickableLabel::paintEvent(QPaintEvent *event){

    if( selected || alwaysSelected ) {

        linearGrad->setColorAt(0, "#FF2F92");
        linearGrad->setColorAt(1, "#00FDFF");
        QPainterPath path;
        path.addRoundedRect(3, 3, 86, 86, 86, 86);
        QPen pen(QBrush(*linearGrad), 3);
        QPainter painter2(this);
        painter2.setRenderHint(QPainter::Antialiasing);
        painter2.setPen(pen);
        painter2.drawPath(path);

    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(*brush);
    painter.drawRoundedRect(11, 10, 70, 70, 70, 70);

}

ClickableLabel::~ClickableLabel() {
    delete brush;
    delete linearGrad;
}

void ClickableLabel::mousePressEvent(QMouseEvent* event) {

    if ( alwaysSelected ) return;
    if( selected == false ) {
        emit clicked(objectName());
        selected = true;

    } else {
        selected = false;
        emit unclicked(objectName());
    }

    repaint();

}

