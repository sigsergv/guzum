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
#include <QtDebug>
#include <QtDBus/QDBusConnection>

#include "settings.h"
#include "encryptedtextwindow.h"
#include "gpgmewrapper.h"
#include "traymanager.h"
#include "traymenuadaptor.h"

int main(int argv, char *_args[])
{
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QApplication app(argv, _args);

    // load localization
    QTranslator translator;
    translator.load("guzum_" + Guzum::Config::uiLang(), Guzum::Config::uiLangsPath());
    app.installTranslator(&translator);
    app.setQuitOnLastWindowClosed(true);

    // parse command line to determine what to do
    // available command line keys:
    // --tray : start as tray icon (prevent multiple instances)
    // filepath : open file in the editor
    QStringList args = QCoreApplication::arguments();
    if (args.length() == 1) {
        // TODO: display file selector
        return 0;
    }

    EncryptedTextWindow * textWindow = 0;

    if (args.length() == 2) {
        if (args[1] == "--tray") {
            // launch in tray icon mode
            qDebug() << "tray icon mode";
            // init settings: tray icon mode
            Guzum::Config::initSettings("icon.ini");
            TrayManager * tray = new TrayManager();

            new TrayMenuAdaptor(tray);
            QDBusConnection connection = QDBusConnection::sessionBus();
            connection.registerObject("/Tray", tray);
            connection.registerService("com.regolit.guzum.tray");
        } else {
            // treat args[1] as a filename
            // we need to check is file exists and create viewer window
            // init settings: display file mode
            Guzum::Config::initSettings("guzum.ini");
            qDebug() << "load file contents mode";
            QFileInfo fi(args[1]);
            if (!fi.exists()) {
                QMessageBox::critical(0, EncryptedTextWindow::tr("Error"), 
                        EncryptedTextWindow::tr("File `%1' not found").arg(args[1]));
                return 1;
            }
            QString filename = fi.canonicalFilePath();
            textWindow = new EncryptedTextWindow(filename);
        }
    }

    if (textWindow) {
        // init gpgme
        GPGME_Error err = GPGME::init();
        if (err != GPG_ERR_NO_ERROR) {
            QMessageBox::critical(0, EncryptedTextWindow::tr("Error"),
                    EncryptedTextWindow::tr("Cannot initialize GPG backend"));
            return 1;
        }
        QApplication::setActiveWindow(textWindow);
        if (!textWindow->show()) {
            return 1;
        }
    }


    /*
    QSqlError se = Guzum::Db::init();
    if (se.type() != QSqlError::NoError) {
        qCritical() << "database connection error:" << se.databaseText() << "\n";
        qCritical() << se.driverText();
        exit(1);
    }
    */

    return app.exec();    
}
