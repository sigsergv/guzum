/*
 * main.cpp
 *
 * Created on: Apr 19, 2012
 * Author: Sergey Stolyarov
 */

#define QT_USE_FAST_CONCATENATION
#define QT_USE_FAST_OPERATOR_PLUS

#include <QtWidgets>
#include <QtDebug>

#include "settings.h"
#include "controlpeer.h"
#include "traymanager.h"

int main(int argv, char *_args[])
{
    QApplication app(argv, _args);

#ifdef Q_OS_MAC
    setenv("PATH", "/usr/local/bin:/bin:/usr/bin", 1);
#endif

    QStringList args = QCoreApplication::arguments();
    Guzum::Config::initSettings();

    ControlPeer * controlPeer = ControlPeer::instance();

    if (controlPeer->mode() == ControlPeer::ModeServer) {
        // initialize notification area icon
        TrayManager::instance();
    }

    switch (controlPeer->mode()) {
    case ControlPeer::ModeUndefined:
        qWarning("Cannot initialize peer, terminating");
        return 1;
        break;

    case ControlPeer::ModeClient:
    case ControlPeer::ModeServer:
        if (args.contains("--select-file")) {
            controlPeer->showFileSelectorDialog();
        } else if (args.length() == 2) {
            controlPeer->editFile(args[1]);
        } else if (args.length() == 3) {
            controlPeer->editFile(args[1], args[2]);
        }
        break;
    }

    if (controlPeer->mode() == ControlPeer::ModeClient) {
        return 0;
    }

    app.setQuitOnLastWindowClosed(false);

    qDebug() << "guzum successfully initialized.";

    return app.exec();    
}
