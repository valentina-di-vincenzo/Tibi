/*
 * Copyright (c) 2020 Valentina Di Vincenzo ( hello@valentina-divincenzo.com )
 *  This file is part of Tibi which is released under the GNU General Public License, version 3.0.
 * See file LICENSE or go to http://www.gnu.org/licenses/ for full license details.
 *
 */

#ifndef TibiSelector_H
#define TibiSelector_H

#include <QObject>
#include <QFile>
#include <QDebug>
#include <QFileInfo>
#include <QtNetwork>

#include <QDataStream>
#include <QTcpSocket>
#include <QLocalServer>


class TibiSelector : public QLocalServer
{
    Q_OBJECT

public:
    TibiSelector() {}

public slots:
    void startSelector();
    void endSelectionSession();
    void closeSelector();
signals:
    void newFileSelection(const QVector<QString>& filePaths);
    void selectorClosed();

private:

    QDataStream in;
    QLocalSocket* selectorSocket;
    QVector<QString> selectedFiles;


protected:
    void incomingConnection(quintptr socketDescriptor) override;


};

#endif // TibiSelector_H
