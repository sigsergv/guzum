/*
 * main.cpp
 *
 * Created on: Apr 19, 2012
 * Author: Sergey Stolyarov
 */

#define QT_USE_FAST_CONCATENATION
#define QT_USE_FAST_OPERATOR_PLUS

#include <QtCore>
#include <QtGui>
#include <QtNetwork>
#include <QtDebug>
#include <QApplication>

#include "settings.h"
#include "encryptedtextwindow.h"
#include "traymanager.h"
#include "controlpeer.h"

#include "iostream"

enum Mode {ModeNone, ModeEditFile, ModeTrayService} ;

void help(QString program)
{
    std::cout << "Usage:\n    " << program.toStdString() << " [args] [FILENAME] [GNUPGHOME]\n";
    std::cout << "Arguments:\n";
    std::cout << "    --help, -h        show this help\n";
    std::cout << "    --select-file     show file selection dialog\n";
    std::cout << "    [FILENAME]        open this encrypted file in the editor\n";
    std::cout << "    [GNUPGHOME]       optional GNUPGHOME directory\n";
    std::cout << std::endl;
}

int main(int argv, char *_args[])
{
    QApplication app(argv, _args);

    // load localization
    QTranslator translator;
    translator.load("guzum_" + Guzum::Config::uiLang(), Guzum::Config::uiLangsPath());
    app.installTranslator(&translator);

    // parse command line to determine what to do
    // available command line keys:
    // --tray : start as tray icon (prevent multiple instances)
    // filepath : open file in the editor
    QStringList args = QCoreApplication::arguments();
    Guzum::Config::initSettings();

    if (args.contains("-h") || args.contains("--help")) {
        help(args[0]);
        return 0;
    }

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

    return app.exec();    
}
