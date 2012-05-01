/*
 * aboutdialog.cpp
 *
 * Created on: May 01, 2012
 * Author: Sergei Stolyarov
 */

#include <QtDebug>

#include "aboutdialog.h"
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
}

AboutDialog::~AboutDialog()
{
    delete p;
}
