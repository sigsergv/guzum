/*
 * prefsdialog.cpp
 *
 * Created on: May 01, 2012
 * Author: Sergey Stolyarov
 */

#include <QtDebug>
#include <QtCore>

#include "prefsdialog.h"
#include "macos.h"
#include "settings.h"
#include "ui_prefsdialog.h"

struct PrefsDialog::Private
{
    Ui::PrefsDialog ui;
    QString autorunDesktop;
    QString autostartDesktopFilePath;
};

PrefsDialog::PrefsDialog(QWidget * parent, Qt::WindowFlags f)
    : QDialog(parent, f)
{
    p = new Private;
    p->ui.setupUi(this);

    // fill uiLanguageCombo with available languages
    p->ui.uiLanguageCombo->addItem("English", "en");
    p->ui.uiLanguageCombo->addItem("Русский", "ru");

    // load desktop file template from the resources
    QFile file(":/guzum-template.desktop");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString tpl = QString::fromUtf8(file.readAll().data());
        file.close();
        // do some replacements
        p->autorunDesktop = tpl.arg(QCoreApplication::applicationFilePath());
    }

    setWindowIcon(QIcon(":/guzum-16.png"));
    // detect is guzum in autostart mode: check file "~/.config/autostart/guzum.desktop"
    auto homeDir = QDir::home();
    
    // try to create dir, check later
    QString path = ".config/autostart";
    homeDir.mkpath(path);
    if (homeDir.exists(path)) {
        p->ui.autostartupCbErrorLabel->hide();
        p->autostartDesktopFilePath = homeDir.path()+"/"+path+"/guzum.desktop";
        QFile guzumDesktopFile(p->autostartDesktopFilePath);
        if (guzumDesktopFile.exists() && 
                guzumDesktopFile.open(QIODevice::ReadOnly | QIODevice::Text)) 
        {
            QString existingAutostartDesktop = QString::fromUtf8(guzumDesktopFile.readAll().data());
            guzumDesktopFile.close();

            if (existingAutostartDesktop == p->autorunDesktop) {
                // simple check: assume autoload is enabled if target .desktop file
                // is exactly the same as we created a few sourcelines ago
                p->ui.autoStartCheckbox->setChecked(true);
            }
        }
    } else {
        // disable checkbox
        p->ui.autoStartCheckbox->setDisabled(true);
    }

    auto lang = Guzum::Config::uiLang();
    auto index = p->ui.uiLanguageCombo->findData(lang);
    p->ui.uiLanguageCombo->setCurrentIndex(index);

    // connect actions
    connect(p->ui.autoStartCheckbox, SIGNAL(stateChanged(int)),
        this, SLOT(setAutostartToggle(int)));
    connect(p->ui.uiLanguageCombo, SIGNAL(activated(int)), 
        this, SLOT(uiLanguageChanged(int)));

#ifdef Q_OS_MAC
    // show app icon in osx dock
    setDockIconStyle(false);
#endif
}

PrefsDialog::~PrefsDialog()
{
    delete p;
}

int PrefsDialog::exec()
{
    QDialog::show();
    // bring window to front
    raise();
    activateWindow();
    // prepare dialog: set checkboxes etc
    return QDialog::exec();
}

void PrefsDialog::setAutostartToggle(int state)
{
    if (p->autostartDesktopFilePath.isEmpty()) {
        return;
    }
    QFile file(p->autostartDesktopFilePath);

    if (state == Qt::Unchecked) {
        // delete file
        file.remove();
    } else {
        // create file
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write(p->autorunDesktop.toUtf8());
            file.close();
        }
    }
}

void PrefsDialog::uiLanguageChanged(int index)
{
    auto lang = p->ui.uiLanguageCombo->itemData(index).toString();
    Guzum::Config::setUiLang(lang);
}