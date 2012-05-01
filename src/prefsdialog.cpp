/*
 * prefsdialog.cpp
 *
 * Created on: May 01, 2012
 * Author: Sergei Stolyarov
 */

#include <QtDebug>
#include <QtCore>

#include "prefsdialog.h"

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
    QDir homeDir = QDir::home();
    
    // try to create dir, don't check
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
                p->ui.autoStartCheckbox->setChecked(true);
            }
        }
    } else {
        // disable checkbox
        p->ui.autoStartCheckbox->setDisabled(true);
    }

    // connect actions
    connect(p->ui.autoStartCheckbox, SIGNAL(stateChanged(int)),
            this, SLOT(setAutostartToggle(int)));

}

PrefsDialog::~PrefsDialog()
{
    delete p;
}

int PrefsDialog::exec()
{
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
