/*
 * traymanager.cpp
 *
 * Created on: Apr 29, 2012
 * Author: Sergei Stolyarov
 */

#include <QtDebug>
#include <QtGui>

#include "traymanager.h"
#include "settings.h"

#define TRAY_FILENAMES_LIST_SIZE_LIMIT 10

struct TrayManager::Private {
    QSystemTrayIcon * trayIcon;
    QMenu * trayMenu;
    QAction * filenamesSeparatorAction;
    QStringList trayFilenames;
    QList<QAction*> trayFilenamesActions;
};

TrayManager::TrayManager(QObject * parent)
    : QObject(parent)
{
    p = new Private;

    p->trayMenu = new QMenu();

    // create actions
    QAction * quitAction = new QAction(tr("&Quit Guzum"), this);

    p->filenamesSeparatorAction = p->trayMenu->addSeparator();
    p->trayMenu->addAction(quitAction);

    // connect signals
    connect(quitAction, SIGNAL(triggered()),
            this, SLOT(quit()));

    // create system tray icon
    p->trayIcon = new QSystemTrayIcon(QIcon(":/guzum-16.png"), this);
    p->trayIcon->setContextMenu(p->trayMenu);
    p->trayIcon->show();

    QSettings * settings = Guzum::Config::settings();
    settings->beginGroup("TrayMenuItems");
    Q_FOREACH (const QString key, settings->allKeys()) {
        QVariant v = settings->value(key);
        if (v.canConvert< QHash<QString, QVariant> >()) {
            QHash<QString, QVariant> value = v.toHash();
            if (value.contains("filename")) {
                QVariant vf = value["filename"];
                if (vf.canConvert<QString>()) {
                    p->trayFilenames << vf.toString();
                }
            }
            //QHash<QString, QString> value = v.con
        }
    }
    settings->endGroup();
    rebuildFilenamesMenu();
    qDebug() << "filenames" << p->trayFilenames;
}

TrayManager::~TrayManager()
{
    delete p;
}

void TrayManager::quit()
{
    QCoreApplication::quit();
}

void TrayManager::appendFile(const QString & filename)
{
    qDebug() << "Append file" << filename << "to the list";
    // first try to find filename in the list and pop it to the top if there is one
    int ind = p->trayFilenames.indexOf(filename);
    if (ind != -1) {
        p->trayFilenames.removeAt(ind);
    }
    p->trayFilenames.insert(0, filename);
    if (p->trayFilenames.size() > TRAY_FILENAMES_LIST_SIZE_LIMIT) {
        // delete the last
        p->trayFilenames.removeAt(p->trayFilenames.size() - 1);
    }
    dumpFilenames();
    rebuildFilenamesMenu();

    qDebug() << p->trayFilenames;
}

void TrayManager::openFilename()
{
    qDebug() << "open filename";
}

void TrayManager::dumpFilenames()
{
    QSettings * settings = Guzum::Config::settings();
    settings->beginGroup("TrayMenuItems");
    settings->remove(""); // remove all keys in the group
    int n = 0;
    QString key;
    Q_FOREACH (const QString & filename, p->trayFilenames) {
        n++;
        key = QString("item-%1").arg(n, 2, 10, QLatin1Char('0'));
        QHash<QString, QVariant> value;
        value["filename"] = filename;
        settings->setValue(key, value);
    }
    settings->endGroup();
    settings->sync();
}

void TrayManager::rebuildFilenamesMenu()
{
    // delete all filename items from tray menu and add again from the list
    QAction * action;
    Q_FOREACH (action, p->trayFilenamesActions) {
        p->trayMenu->removeAction(action);
        delete action;
    }
    p->trayFilenamesActions.clear();

    Q_FOREACH (const QString & filename, p->trayFilenames) {
        action = new QAction(filename, this);
        connect(action, SIGNAL(triggered()),
                this, SLOT(openFilename()));
        p->trayFilenamesActions.append(action);
    }

    p->trayMenu->insertActions(p->filenamesSeparatorAction, p->trayFilenamesActions);
}
