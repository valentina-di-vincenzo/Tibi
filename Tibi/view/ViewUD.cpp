/*
 * Copyright (c) 2020 Valentina Di Vincenzo ( hello@valentina-divincenzo.com )
 *  This file is part of Tibi which is released under the GNU General Public License, version 3.0.
 * See file LICENSE or go to http://www.gnu.org/licenses/ for full license details.
 *
 */

#include "ViewUD.h"

ViewUD::ViewUD()
{

    uploadsLayout = new QGridLayout;
    downloadsLayout = new QGridLayout;
    uploadRows = new QMap<int, UDRow*>;
    downloadRows = new QMap<int, UDRow*>;
    createSendingWindow();

}


void ViewUD::createSendingWindow() {
    QSize size = qApp->screens()[0]->size();
    setFixedHeight(size.height() - 80);
    setMinimumWidth(900);
    setWindowTitle("Tibi is sharing..");

    QVBoxLayout* upLayout = createCentralLayout("uploads", "T O", uploadsLayout);
    QVBoxLayout* downLayout = createCentralLayout("downloads", "F R O M", downloadsLayout);
    QVBoxLayout* centralLayout = new QVBoxLayout;
    centralLayout->addLayout(upLayout);
    centralLayout->addSpacing(30);
    centralLayout->addLayout(downLayout);

    QWidget *centralWidget = new QWidget;
    centralWidget->setLayout(centralLayout);
    centralWidget->setContentsMargins(20, 10, 20, 10);
    setCentralWidget(centralWidget);

}

QVBoxLayout* ViewUD::createCentralLayout(const QString& title, const QString& toOrFrom, QGridLayout* layout) {

    QHBoxLayout* titleLayout = createTitleLayout(title);
    QHBoxLayout* infoLayout = createInfoLayout(toOrFrom);
    QLabel* separatorLine = createSeparator();
    QScrollArea* scroll = createScrollArea(layout);
    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addLayout(titleLayout);
    centralLayout->setAlignment(titleLayout, Qt::AlignLeft);
    centralLayout->addSpacing(10);
    centralLayout->addLayout(infoLayout);
    centralLayout->setAlignment(infoLayout, Qt::AlignLeft);
    centralLayout->addWidget(separatorLine);
    centralLayout->setAlignment(separatorLine, Qt::AlignLeft);
    centralLayout->addWidget(scroll);
    centralLayout->setSpacing(20);

    return centralLayout;
}

QLabel* ViewUD::createLogo() {
    QLabel* logo = new QLabel;
    QPixmap pixmapLogo(":/images/icons/tibi-icon.png");
    pixmapLogo = pixmapLogo.scaled(50, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    logo->setPixmap(pixmapLogo);
    return logo;
}

QLabel* ViewUD::createTitle(const QString& title) {
    QLabel* titleLabel = new QLabel;
    titleLabel->setText(title);
    titleLabel->setObjectName("title");
    return titleLabel;
}

QHBoxLayout* ViewUD::createTitleLayout(const QString& title) {
    QLabel* iconLogo = createLogo();
    QLabel* titleLabel = createTitle(title);
    QHBoxLayout* titleLayout = new QHBoxLayout;
    titleLayout->addWidget(iconLogo);
    titleLayout->addWidget(titleLabel);
    titleLayout->setSpacing(20);
    return titleLayout;
}

QHBoxLayout* ViewUD::createInfoLayout(const QString& toOrFrom) {
    QLabel* to = new QLabel(toOrFrom);
    QLabel* filename = new QLabel("W H A T");
    QLabel* time = new QLabel("S T A T U S");
    to->setObjectName("firstRow");
    filename->setObjectName("firstRow");
    time->setObjectName("firstRow");
    to->setFixedWidth(150);
    filename->setFixedWidth(300);
    time->setFixedWidth(100);

    QHBoxLayout* infoLayout = new QHBoxLayout;
    infoLayout->addWidget(to);
    infoLayout->setAlignment(to, Qt::AlignLeft);
    infoLayout->addWidget(filename);
    infoLayout->setAlignment(filename, Qt::AlignLeft);
    infoLayout->addWidget(time);
    infoLayout->setAlignment(time, Qt::AlignLeft);

    return infoLayout;
}

QLabel* ViewUD::createSeparator() {
    QPixmap pix( QSize(width(), 2));
    pix.fill(Qt::gray);
    QLabel* lineLabel = new QLabel;
    lineLabel->setPixmap(pix);
    return lineLabel;
}

QScrollArea* ViewUD::createScrollArea(QGridLayout *layout) {
    layout->setAlignment(Qt::AlignTop);
    QWidget* wrapperWidget = new QWidget;
    wrapperWidget->setObjectName("uploads");
    wrapperWidget->setLayout(layout);
    QScrollArea* scroll = new QScrollArea;
    scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setStyleSheet("border: 0;");
    scroll->setWidget(wrapperWidget);
    return scroll;
}

void ViewUD::addUD(bool upload, int code, const QString& tibierName, const QString& fileName) {
    qDebug() << "add UD";
    QGridLayout* layout = uploadsLayout;
    QMap<int, UDRow*>* map = uploadRows;
    if( ! upload ) {
        layout = downloadsLayout;
        map = downloadRows;
    }

    UDRow* row = new UDRow(code, tibierName, fileName);
    map->insert(code, row);
    int rowCount = layout->rowCount()+1;
    layout->addWidget(row->to, rowCount, 0);
    layout->addWidget(row->name, rowCount, 1);
    layout->addWidget(row->time, rowCount, 2);
    layout->addWidget(row->bar, rowCount, 3);
    layout->addWidget(row->abort, rowCount, 4);
    connect(row->abort, &QPushButton::clicked, this, [=]() {
        upload?
                    emit signalUploadAbort(row->code) : emit signalDownloadAbort(row->code);
        row->time->setText("ABORTING..");
        row->lastTime = QTime(0,0,0);
        row->bar->setRange(0,0);
        row->bar->update();
        row->abort->setVisible(false);
    });

    raiseView();

}



void ViewUD::updateUD(bool upload, int code, int perc, const QTime& time, const QString& info, const QString& path) {
    QGridLayout* layout = uploadsLayout;
    QMap<int, UDRow*>* map = uploadRows;
    if( ! upload ) {
        layout = downloadsLayout;
        map = downloadRows;
    }

    //qDebug() << "[ VIEW ] NEW UPDATE " << perc << "%";


    if( !map->contains(code)) {
        qDebug() << "There is no corresponding code!";
        return;
    }
    UDRow* row = map->value(code);
    row->bar->setValue(perc);
    row->bar->setRange(0,100);

    if(info == "") {
        row->lastTime = time;
        row->time->setText( "- "
                            + (time.hour() < 10 ? "0" + QString::number(time.hour()) :  QString::number(time.hour())) + " h "
                            + (time.minute() < 10 ? "0" + QString::number(time.minute()) :  QString::number(time.minute())) + " m "
                            + (time.second() < 10 ? "0" + QString::number(time.second()) :  QString::number(time.second())) + " s");
    }

    else {
        row->lastTime = QTime(0,0,0);
        row->time->setText(info);
        if( info != "WAITING" ) row->time->setStyleSheet("color: #FF2F92");
    }

    if(perc == 0) {
        row->bar->setRange(0,0);
        row->bar->setStyleSheet("background-color: #212121;");

    }

    if(perc >= 100) {
        completed(map, layout, row);
        if( info == "COMPLETED") {
            if( upload ) {
                row->time->setStyleSheet("color: #00FDFF");
            }

            else {
                row->open = new QPushButton("open");
                row->open->setFixedWidth(70);
                connect(row->open, &QPushButton::clicked, this, [=]() {
                    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
                    layout->invalidate();
                });
                int rowIndex;
                int columnIndex;
                int spanRow;
                int spanColumn;
                layout->getItemPosition(layout->indexOf(row->time), &rowIndex, &columnIndex, &spanRow, &spanColumn);
                layout->addWidget(row->open, rowIndex, columnIndex);
                layout->setAlignment(row->open, Qt::AlignLeft);
                row->time->setVisible(false);
            }
        }
    }

    row->bar->update();


}

void ViewUD::updateDownloadName(int code, const QString& newName) {
    if( downloadRows->contains(code)) {
        UDRow* row = downloadRows->value(code);
        row->name->setText(newName);
        downloadsLayout->invalidate();
    }
}

void ViewUD::completed(QMap<int, UDRow*>* map, QGridLayout* layout, UDRow* row) {

    row->bar->setStyleSheet("background-color: #212121;");
    row->bar->setRange(0,100);
    row->bar->setValue(0);
    row->abort->setText("remove");
    row->abort->disconnect();
    connect(row->abort, &QPushButton::clicked, this, [=]() {
        removeItem(map, layout, row);
    });
    row->abort->setVisible(true);


}

void ViewUD::removeItem(QMap<int, UDRow*>* map, QGridLayout* layout, UDRow* row) {
    map->remove(row->code);
    layout->invalidate();
    delete row;
}


void ViewUD::raiseView() {
    this->showNormal();
    this->raise();
}


ViewUD::~ViewUD() {

    for( UDRow* row : *uploadRows ) {
        if( row != nullptr ) delete row;
    }
    delete uploadRows;

    for( UDRow* row : *downloadRows ) {
        if( row != nullptr ) delete row;
    }
    delete downloadRows;
}
