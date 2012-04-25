/*
 * passphrasedialog.cpp
 *
 * Created on: Apr 25, 2012
 * Author: Sergei Stolyarov
 */

#include <QtDebug>

#include "passphrasedialog.h"
#include "ui_passphrasedialog.h"

struct PassphraseDialog::Private
{
    Ui::PassphraseDialog ui;
};

PassphraseDialog::PassphraseDialog(QWidget * parent, Qt::WindowFlags f)
    :QDialog(parent, f)
{
    p = new Private;
    p->ui.setupUi(this);
    p->ui.errorLabel->hide();
}

PassphraseDialog::~PassphraseDialog()
{
    delete p;
}

QString PassphraseDialog::passphrase()
{
    return p->ui.passLineEdit->text();
}

void PassphraseDialog::setUid(const QString & uid)
{
    p->ui.uidLabel->setText(uid);
}

void PassphraseDialog::triggerErrorLabel()
{
    p->ui.errorLabel->show();
}
