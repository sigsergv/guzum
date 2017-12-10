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
}

AboutDialog::~AboutDialog()
{
    delete p;
}
