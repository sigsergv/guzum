/*
 * traymanager.cpp
 *
 * Created on: Apr 29, 2012
 * Author: Sergey Stolyarov
 */

#include <QtDebug>
#include <QtWidgets>

#include "traymanager.h"
#include "macos.h"
#include "prefsdialog.h"
#include "managehistorydialog.h"
#include "aboutdialog.h"
#include "controlpeer.h"
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

TrayManager * TrayManager::inst = 0;

TrayManager::TrayManager(QObject * parent)
    : QObject(parent)
{
    p = new Private;

    p->trayMenu = new QMenu();

    // create actions
    auto manageHistoryAction = new QAction(tr("&Manage history"), this);
    auto selectFileDialogAction = new QAction(tr("&Open file…"), this);
    auto prefsAction = new QAction(tr("&Preferences…"), this);
    auto aboutAction = new QAction(tr("&About Guzum"), this);
    auto quitAction = new QAction(tr("&Quit Guzum"), this);

    // create menu items
    p->filenamesSeparatorAction = p->trayMenu->addSeparator();
    p->trayMenu->addAction(manageHistoryAction);
    p->trayMenu->addSeparator();
    p->trayMenu->addAction(selectFileDialogAction);
    p->trayMenu->addAction(prefsAction);
    p->trayMenu->addAction(aboutAction);
    p->trayMenu->addAction(quitAction);

    // connect signals
    connect(manageHistoryAction, SIGNAL(triggered()),
            this, SLOT(manageHistory()));
    connect(prefsAction, SIGNAL(triggered()),
            this, SLOT(setPreferences()));
    connect(aboutAction, SIGNAL(triggered()),
            this, SLOT(showAboutDialog()));
    connect(quitAction, SIGNAL(triggered()),
            this, SLOT(quit()));
    connect(selectFileDialogAction, SIGNAL(triggered()),
            this, SLOT(selectFilename()));

    auto app = Guzum::Config::app();
    connect(app, SIGNAL(lastWindowClosed()),
        this, SLOT(lastWindowClosed()));
    
    // for future use
    //connect(p->trayMenu, SIGNAL(hovered(QAction*)),
    //        this, SLOT(menuHovered(QAction*)));

    // create system tray icon
    p->trayIcon = new QSystemTrayIcon(QIcon(":/guzum-16.png"), this);
    p->trayIcon->setContextMenu(p->trayMenu);
    p->trayIcon->show();

    reloadFilenames();
    rebuildFilenamesMenu();
}

TrayManager * TrayManager::instance()
{
    if (inst == 0) {
        inst = new TrayManager();
    }

    return inst;
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
    // first try to find filename in the list and pop it to the top if there is one
    auto len = p->trayFilenames.size();
    auto ind = -1;
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
    dumpFilenames();
    rebuildFilenamesMenu();
}

void TrayManager::openFilename()
{
    // first find what action has been triggered, extract filename and open it
    auto action = qobject_cast<QAction*>(sender());
    auto item = action->data().toHash();

    auto filename = item["filename"].toString();
    auto gnupgHome = item["gnupghome"].toString();

    auto peer = ControlPeer::instance();
    peer->editFile(filename, gnupgHome);
}

void TrayManager::selectFilename()
{
    auto peer = ControlPeer::instance();
    peer->showFileSelectorDialog();
}

void TrayManager::setPreferences()
{
    PrefsDialog dlg;
    dlg.exec();
}

void TrayManager::showAboutDialog()
{
    AboutDialog dlg;
    dlg.exec();
}

void TrayManager::manageHistory()
{
    ManageHistoryDialog dlg;
    dlg.exec();
    reloadFilenames();
    rebuildFilenamesMenu();
}

void TrayManager::menuHovered(QAction * action)
{
    // find action first
    auto found = false;
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
    auto settings = Guzum::Config::settings();
    settings->beginGroup("TrayMenuItems");
    settings->remove(""); // remove all keys in the group
    auto n = 0;
    Q_FOREACH (const QStringsHash item, p->trayFilenames) {
        n++;
        auto key = QString("item-%1").arg(n, 2, 10, QLatin1Char('0')); // form name like "item-64"
        settings->setValue(key, item);
    }
    settings->endGroup();
    settings->sync();
}

void TrayManager::reloadFilenames()
{
    p->trayFilenames.clear();

    auto settings = Guzum::Config::settings();
    settings->beginGroup("TrayMenuItems");
    Q_FOREACH (const QString key, settings->allKeys()) {
        auto v = settings->value(key);
        if (v.canConvert< QHash<QString, QVariant> >()) {
            QHash<QString, QVariant> value = v.toHash();
            if (value.contains("filename")) {
                auto vf = value["filename"];
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
}

void TrayManager::lastWindowClosed()
{
#ifdef Q_OS_MAC
    // hide dock icon
    setDockIconStyle(true);
#endif
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
        action->setData(item);
        connect(action, SIGNAL(triggered()),
                this, SLOT(openFilename()));
        p->trayFilenamesActions.append(action);
    }

    p->trayMenu->insertActions(p->filenamesSeparatorAction, p->trayFilenamesActions);
}
