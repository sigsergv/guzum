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

#include "iostream"

enum Mode {ModeNone, ModeEditFile, ModeTrayService} ;

int startFileEdit(QString filename)
{
    EncryptedTextWindow * textWindow;
    QFileInfo fi(filename);
    if (!fi.exists()) {
        QMessageBox::critical(0, EncryptedTextWindow::tr("Error"), 
                EncryptedTextWindow::tr("File `%1' not found").arg(filename));
        return 1;
    }
    filename = fi.canonicalFilePath();
    textWindow = new EncryptedTextWindow(filename);

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
    return 0;
}

void help(QString program)
{
    std::cout << "Usage:\n    " << program.toStdString() << " [args] [FILENAME]\n";
    std::cout << "Arguments:\n";
    std::cout << "    --help, -h   show this help\n";
    std::cout << "    --tray       launch as icon in the notification area\n";
    std::cout << "    --dialog     show file selection dialog";
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

    QString editFilename;
    Mode mode = ModeNone;

    if (args.contains("-h") || args.contains("--help")) {
        help(args[0]);
        return 0;
    }
    if (args.contains("--tray")) {
        mode = ModeTrayService;
        app.setQuitOnLastWindowClosed(false);
    } else if (args.contains("--dialog")) {
        mode = ModeEditFile;
        // show file selector
        Guzum::Config::initSettings("guzum.ini");
        // read last used dir name from the settings
        QSettings * settings = Guzum::Config::settings();
        settings->beginGroup("EditFileSelector");
        QString initDir = settings->value("init-dir").toString();
        settings->endGroup();

        if (initDir.isEmpty()) {
            initDir = QDir::homePath();
        }
        editFilename = QFileDialog::getOpenFileName(0, EncryptedTextWindow::tr("Select file encrypted by Gnupg"), 
            initDir,
            EncryptedTextWindow::tr("Encrypted files (*.gpg, *.asc) (*.gpg *.asc);;All files (*.*)"), 
            0, 0);
        if (editFilename.isEmpty()) {
            return 0;
        }
        // remember last used directory
        QFileInfo fi(editFilename);
        settings->beginGroup("EditFileSelector");
        settings->setValue("init-dir", fi.canonicalPath());
        settings->endGroup();
        settings->sync();
    } else if (args.length() == 2) {
        mode = ModeEditFile; 
        app.setQuitOnLastWindowClosed(false);
        Guzum::Config::initSettings("guzum.ini");
        editFilename = args[1];
    } else {
        help(args[0]);
        exit(0);
    }

    switch (mode) {
    case ModeEditFile: {
        // treat args[1] as a filename
        // we need to check is file exists and create viewer window
        // init settings: display file mode
        app.setQuitOnLastWindowClosed(true);
        qDebug() << "load file contents mode";
        int res = startFileEdit(editFilename);
        if (res != 0) {
            return res;
        }
        };
        break;

    case ModeTrayService: {
        // launch in tray icon mode
        qDebug() << "tray icon mode";
        // init settings: tray icon mode
        Guzum::Config::initSettings("icon.ini");
        TrayManager * tray = new TrayManager();

        new TrayMenuAdaptor(tray);
        QDBusConnection connection = QDBusConnection::sessionBus();
        connection.registerObject("/Tray", tray);
        connection.registerService("com.regolit.guzum.tray");
        };
        break;

    default:
        return 0;
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
