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

#include "settings.h"
#include "encryptedtextwindow.h"

int main(int argv, char *_args[])
{
    // init settings
    Guzum::Config::settings();

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
        // display file selector
        return 0;
    }

    EncryptedTextWindow * textWindow = 0;

    if (args.length() == 2) {
        if (args[1] == "--tray") {
            // launch in tray icon mode
            qDebug() << "tray icon mode";
        } else {
            // treat args[1] as a filename
            // we need to check is file exists and create viewer window
            qDebug() << "load file contents mode";
            QFileInfo f(args[1]);
            if (!f.exists()) {
                QMessageBox::critical(0, EncryptedTextWindow::tr("Error"), 
                        EncryptedTextWindow::tr("File `%1' not found").arg(args[1]));
                return 1;
            }
            QString filename = f.canonicalFilePath();
            qDebug() << filename;
            textWindow = new EncryptedTextWindow(filename);
        }
    }

    if (textWindow) {
        QApplication::setActiveWindow(textWindow);
        textWindow->show();
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
