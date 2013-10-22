/*
 * main.cpp
 *
 * Created on: Apr 19, 2012
 * Author: Sergei Stolyarov
 */

#define QT_USE_FAST_CONCATENATION
#define QT_USE_FAST_OPERATOR_PLUS

#include <QtCore>
#include <QtGui>
#include <QtNetwork>
#include <QtDebug>
#include <QtDBus/QDBusConnection>

#include "settings.h"
#include "encryptedtextwindow.h"
#include "traymanager.h"
#include "controlpeer.h"

#include "iostream"

enum Mode {ModeNone, ModeEditFile, ModeTrayService} ;

void help(QString program)
{
    std::cout << "Usage:\n    " << program.toStdString() << " [args] [FILENAME]\n";
    std::cout << "Arguments:\n";
    std::cout << "    --help, -h        show this help\n";
    std::cout << "    --select-file     show file selection dialog\n";
    std::cout << "    [FILENAME]        open this encrypted file in the editor\n";
    std::cout << std::endl;
}

int main(int argv, char *_args[])
{
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
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
    Guzum::Config::initSettings("guzum.ini");

    if (args.contains("-h") || args.contains("--help")) {
        help(args[0]);
        return 0;
    }

    ControlPeer * controlPeer = new ControlPeer();

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
        }
        break;
    }

    if (controlPeer->mode() == ControlPeer::ModeClient) {
        return 0;
    }

    TrayManager * tray = new TrayManager();

    app.setQuitOnLastWindowClosed(false);

    /*
    new TrayMenuAdaptor(tray);
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.registerObject("/Tray", tray);
    connection.registerService("com.regolit.guzum.tray");
    */

    return app.exec();    
}
