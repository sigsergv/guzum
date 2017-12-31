/*
 * aboutdialog.cpp
 *
 * Created on: May 01, 2012
 * Author: Sergey Stolyarov
 */

#include <QtDebug>

#include<QtGui>

#include "aboutdialog.h"
#include "settings.h"
#include "macos.h"
#include "ui_aboutdialog.h"

struct AboutDialog::Private
{
    Ui::AboutDialog ui;
};

AboutDialog::AboutDialog(QWidget * parent, Qt::WindowFlags f)
    : QDialog(parent, f)
{
    p = new Private;
    p->ui.setupUi(this);
    setWindowIcon(QIcon(":/guzum-16.png"));
    p->ui.versionLabel->setText(tr("version %1").arg(GUZUM_VERSION));
    p->ui.qtCompiledVersionLabel->setText(tr("Compiled with Qt %1").arg(qVersion()));
    p->ui.qtVersionLabel->setText(tr("Runtime version %1").arg(QT_VERSION_STR));

#ifdef Q_OS_MAC
    // show app icon in osx dock
    setDockIconStyle(false);
#endif
}

AboutDialog::~AboutDialog()
{
    delete p;
}

int AboutDialog::exec()
{
    QDialog::show();
    // bring window to front
    raise();
    activateWindow();
    return QDialog::exec();
}