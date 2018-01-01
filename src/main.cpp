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
#include "macos.h"
#include <stdio.h>
#include <stdlib.h>

void print_help()
{
    QString msg = "Guzum.\n"
        "Without arguments\n    start in notification area icon mode.\n\n"
        "guzum FILENAME\n    try to open FILENAME in secure editor\n\n"
        "guzum --select-file\n    open file selector\n\n";
    fprintf(stdout, "%s", msg.toLocal8Bit().constData());
}

int main(int argv, char *_args[])
{
    QApplication app(argv, _args);
    qDebug() << "Starting in DEBUG mode";

#ifdef Q_OS_MAC
    // we need to fix PATH and add /usr/local/bin where gpg binary is usually located
    setenv("PATH", "/usr/local/bin:/bin:/usr/bin", 1);
#endif

    auto args = QCoreApplication::arguments();
    if (args.contains("--help") || args.contains("-h")) {
        print_help();
        return 0;
    }

    Guzum::Config::initSettings(&app);

    auto controlPeer = ControlPeer::instance();
    auto controlPeerMode = controlPeer->mode();

    if (controlPeerMode == ControlPeer::ModeServer) {
        // initialize notification area icon
        qDebug("Starting Guzum in server mode");
        TrayManager::instance();
    } else if (controlPeerMode == ControlPeer::ModeClient) {
        qDebug("Starting Guzum in client mode");
    } else {
        qWarning("Cannot initialize peer, terminating");
        return 1;
    }

    switch (controlPeerMode) {
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
    case ControlPeer::ModeUndefined:
        // nothing to do here
        break;
    }

    if (controlPeerMode == ControlPeer::ModeClient) {
        // quit app as we don't do anything in client mode, just
        // passed commands to server and that's enough
        return 0;
    }

    app.setQuitOnLastWindowClosed(false);

#ifdef Q_OS_MAC
    // hide dock icon
    setDockIconStyle(true);
#endif

    return app.exec();    
}
