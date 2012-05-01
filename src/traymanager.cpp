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

typedef QHash<QString, QVariant> QStringsHash;

struct TrayManager::Private {
    QSystemTrayIcon * trayIcon;
    QMenu * trayMenu;
    QAction * filenamesSeparatorAction;
    QList< QStringsHash > trayFilenames;
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
    
    // for future use
    //connect(p->trayMenu, SIGNAL(hovered(QAction*)),
    //        this, SLOT(menuHovered(QAction*)));

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
                QStringsHash item;

                if (vf.canConvert<QString>()) {
                    item["filename"] = vf.toString();
                }
                item["gnupghome"] = QString();
                if (value.contains("gnupghome") && value["gnupghome"].canConvert<QString>()) {
                    // this is a path to gnupg data directory
                    item["gnupghome"] = value["gnupghome"].toString();
                }
                p->trayFilenames << item;
            }
        }
    }
    settings->endGroup();
    rebuildFilenamesMenu();
}

TrayManager::~TrayManager()
{
    delete p;
}

void TrayManager::quit()
{
    QCoreApplication::quit();
}

void TrayManager::appendFile(const QString & filename, const QString & gnupgHome)
{
    qDebug() << "Append file" << filename << "to the list";
    // first try to find filename in the list and pop it to the top if there is one
    int len = p->trayFilenames.size();
    int ind = -1;
    for (int i=0; i<len; i++) {
        QStringsHash item = p->trayFilenames[i];
        if (item["filename"] == filename) {
            ind = i;
            break;
        }
    }
    if (ind != -1) {
        p->trayFilenames.removeAt(ind);
    }
    QStringsHash item;
    item["filename"] = filename;
    item["gnupghome"] = gnupgHome;
    p->trayFilenames.insert(0, item);
    if (p->trayFilenames.size() > TRAY_FILENAMES_LIST_SIZE_LIMIT) {
        // delete the last
        p->trayFilenames.removeAt(p->trayFilenames.size() - 1);
    }
    qDebug() << item;
    dumpFilenames();
    rebuildFilenamesMenu();
}

void TrayManager::openFilename()
{
    // first find what action has been triggered, extract filename and open it
    QAction * action = qobject_cast<QAction*>(sender());
    QStringsHash item = action->data().toHash();

    QString filename = item["filename"].toString();
    QString gnupgHome = item["gnupghome"].toString();
    qDebug() << "filename: " << filename << "home" << gnupgHome;

    // execute the same application but pass filename as the argument
    QString app = QCoreApplication::applicationFilePath();
    QStringList args;
    args << filename;
     
    QProcess::startDetached(app, args);
}

void TrayManager::menuHovered(QAction * action)
{
    // find action first
    bool found = false;
    Q_FOREACH (QAction * a, p->trayFilenamesActions) {
        if (a == action) {
            found = true;
            break;
        }
    }
    if (!found) {
        return;
    }

    QToolTip::showText(QCursor::pos(), action->text(), 0);
}

void TrayManager::dumpFilenames()
{
    QSettings * settings = Guzum::Config::settings();
    settings->beginGroup("TrayMenuItems");
    settings->remove(""); // remove all keys in the group
    int n = 0;
    QString key;
    Q_FOREACH (const QStringsHash item, p->trayFilenames) {
        n++;
        key = QString("item-%1").arg(n, 2, 10, QLatin1Char('0')); // form name like "item-64"
        settings->setValue(key, item);
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

    Q_FOREACH (const QStringsHash item, p->trayFilenames) {
        action = new QAction(item["filename"].toString(), this);
        //action->setToolTip("asd");
        action->setData(item);
        connect(action, SIGNAL(triggered()),
                this, SLOT(openFilename()));
        p->trayFilenamesActions.append(action);
    }

    p->trayMenu->insertActions(p->filenamesSeparatorAction, p->trayFilenamesActions);
}