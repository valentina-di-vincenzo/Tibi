/*
 * Copyright (c) 2020 Valentina Di Vincenzo ( hello@valentina-divincenzo.com )
 *  This file is part of Tibi which is released under the GNU General Public License, version 3.0.
 * See file LICENSE or go to http://www.gnu.org/licenses/ for full license details.
 *
 */

#ifndef TIBIRECEIVER_H
#define TIBIRECEIVER_H

#include <QTcpServer>
#include <QNetworkInterface>

class TibiReceiver : public QTcpServer
{
    Q_OBJECT

public:
    TibiReceiver() : QTcpServer() {}

protected:
    void incomingConnection(qintptr socketDescriptor) override;

public slots:
    void startReceiver();
    void closeReceiver();

signals:
    void receiverRunningAt(QHostAddress address, int port);
    void newDownloadRequest(qintptr socketDescriptor);
    void receiverClosed();

private:
    QHostAddress getIpAddress();

};

#endif // TIBIRECEIVER_H
