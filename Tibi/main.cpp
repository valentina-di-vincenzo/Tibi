/*
 * Copyright (c) 2020 Valentina Di Vincenzo ( hello@valentina-divincenzo.com )
 *  This file is part of Tibi which is released under the GNU General Public License, version 3.0.
 * See file LICENSE or go to http://www.gnu.org/licenses/ for full license details.
 *
 */

#include <QApplication>
#include "TibiMediator.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(tibi);

    QCoreApplication::setOrganizationName("valentina-di-vincenzo");
    QCoreApplication::setApplicationName("Tibi");
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication app(argc, argv);
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(nullptr, QObject::tr("Systray"),
                              QObject::tr("I couldn't detect any system tray "
                                          "on this system."));
        return 1;
    }

    QApplication::setQuitOnLastWindowClosed(false);
    qDebug() << QThread::currentThreadId() << " - MAIN";
    TibiMediator manager;

    return app.exec();
}
