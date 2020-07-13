/*
 * Copyright (c) 2020 Valentina Di Vincenzo ( hello@valentina-divincenzo.com )
 *  This file is part of Tibi which is released under the GNU General Public License, version 3.0.
 * See file LICENSE or go to http://www.gnu.org/licenses/ for full license details.
 * 
 */

#include <QLocalSocket>
#include <QDataStream>

int main(int argc, char *argv[])
{
    qDebug() << "Received " << argc - 1 << " file/folder to send.";

   //1. Mi accerto che argc sia > 1
    if( argc < 2 ) {
        return 1;
    }

   //2. Init local socket and connect to local server
    QLocalSocket selector;
    selector.setServerName("selector");
    selector.connectToServer(QIODevice::WriteOnly);
    if(!selector.waitForConnected(5000)) {
        return 1;
    }


    //3. Send info
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    int selected = argc - 1;
    out << selected;
    for ( int i = 0; i < selected; i++ ) {
        QString selectedPath = QString::fromUtf8(argv[i+1]);
        out << selectedPath;
        qDebug() << selectedPath;
    }
    selector.write(block);
    selector.waitForBytesWritten();
    selector.waitForDisconnected(-1);

}
