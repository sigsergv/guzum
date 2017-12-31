/*
 * managehistorydialog.cpp
 *
 * Created on: May 03, 2012
 * Author: Sergey Stolyarov
 */

#include <QtCore>
#include <QtGui>
#include <QtDebug>

#include "settings.h"
#include "macos.h"
#include "managehistorydialog.h"
#include "ui_managehistorydialog.h"

typedef QHash<QString, QVariant> QStringsHash;

struct ManageHistoryDialog::Private
{
    Ui::ManageHistoryDialog ui;
};

ManageHistoryDialog::ManageHistoryDialog(QWidget * parent, Qt::WindowFlags f)
    : QDialog(parent, f)
{
    p = new Private;
    p->ui.setupUi(this);

    connect(p->ui.itemsListWidget, SIGNAL(itemSelectionChanged()),
            this, SLOT(itemSelectionChanged()));
    connect(p->ui.deleteItemButton, SIGNAL(clicked()),
            this, SLOT(deleteItem()));
    connect(p->ui.filepathLineEdit, SIGNAL(textEdited(const QString &)),
            this, SLOT(editingFilename(const QString &)));
    connect(p->ui.gnupghomeLineEdit, SIGNAL(textEdited(const QString &)),
            this, SLOT(editingGnupgHome(const QString &)));

    p->ui.itemsListWidget->setDragDropMode(QAbstractItemView::InternalMove);

    // load history items to itemsListWidget
    QSettings * settings = Guzum::Config::settings();
    settings->beginGroup("TrayMenuItems");
    Q_FOREACH (const QString key, settings->allKeys()) {
        QVariant v = settings->value(key);
        if (!v.canConvert< QHash<QString, QVariant> >()) {
            continue;
        }
        QHash<QString, QVariant> value = v.toHash();
        if (value.contains("filename")) {
            QVariant vf = value["filename"];
            QListWidgetItem * item = new QListWidgetItem(p->ui.itemsListWidget);
            if (!vf.canConvert<QString>()) {
                continue;
            }
            item->setText(vf.toString());
            item->setData(FilenameRole, vf);

            if (value.contains("gnupghome") && value["gnupghome"].canConvert<QString>()) {
                // this is a path to gnupg data directory
                item->setData(GnupgHomeRole, value["gnupghome"]);
            }
            p->ui.itemsListWidget->addItem(item);
        }
    }
    settings->endGroup();

#ifdef Q_OS_MAC
    // show app icon in osx dock
    setDockIconStyle(false);
#endif
}

ManageHistoryDialog::~ManageHistoryDialog()
{
    delete p;
}

int ManageHistoryDialog::exec()
{
    QDialog::show();
    // bring window to front
    raise();
    activateWindow();
    
    int res = QDialog::exec();
    if (res == QDialog::Accepted) {
        // write items back to the settings
        QSettings * settings = Guzum::Config::settings();
        settings->beginGroup("TrayMenuItems");
        settings->remove("");
        QString key;
        int cnt = p->ui.itemsListWidget->count();
        for (int i=0; i<cnt; i++) {
            QListWidgetItem * item = p->ui.itemsListWidget->item(i);
            QStringsHash h;
            h["filename"] = item->data(FilenameRole);
            h["gnupghome"] = item->data(GnupgHomeRole);
            key = QString("item-%1").arg(i, 2, 10, QLatin1Char('0'));
            settings->setValue(key, h);
        }
        settings->endGroup();
        settings->sync();
    }
    return res;
}


void ManageHistoryDialog::itemSelectionChanged()
{
    QList<QListWidgetItem*> selected = p->ui.itemsListWidget->selectedItems();
    bool disabled = false;

    if (selected.size() == 0) {
        disabled = true;
        p->ui.filepathLineEdit->setText("");
        p->ui.gnupghomeLineEdit->setText("");
    } else {
        QListWidgetItem * item = selected.first();
        p->ui.filepathLineEdit->setText(item->data(FilenameRole).toString());
        p->ui.gnupghomeLineEdit->setText(item->data(GnupgHomeRole).toString());
    }

    p->ui.filepathLineEdit->setDisabled(disabled);
    p->ui.gnupghomeLineEdit->setDisabled(disabled);
    p->ui.deleteItemButton->setDisabled(disabled);
}

void ManageHistoryDialog::deleteItem()
{
    
    QList<QListWidgetItem*> selected = p->ui.itemsListWidget->selectedItems();
    if (selected.size() == 0) {
        return;
    }
    QListWidgetItem * item = selected.first();

    int row = p->ui.itemsListWidget->row(item);
    p->ui.itemsListWidget->takeItem(row);
    delete item;
}

void ManageHistoryDialog::editingFilename(const QString & text)
{
    QList<QListWidgetItem*> selected = p->ui.itemsListWidget->selectedItems();
    if (selected.size() == 0) {
        return;
    }
    QListWidgetItem * item = selected.first();
    item->setText(text);
    item->setData(FilenameRole, text);
}

void ManageHistoryDialog::editingGnupgHome(const QString & text)
{
    QList<QListWidgetItem*> selected = p->ui.itemsListWidget->selectedItems();
    if (selected.size() == 0) {
        return;
    }
    QListWidgetItem * item = selected.first();
    item->setData(GnupgHomeRole, text);
}
