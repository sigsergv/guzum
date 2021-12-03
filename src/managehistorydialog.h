/*
 * managehistorydialog.h
 *
 * Created on: May 03, 2012
 * Author: Sergey Stolyarov
 */

#ifndef _MANAGEHISTORYDIALOG_H_
#define _MANAGEHISTORYDIALOG_H_

#include <QDialog>

class ManageHistoryDialog : public QDialog
{
    Q_OBJECT
public:
    ManageHistoryDialog(QWidget * parent = 0, Qt::WindowFlags f = Qt::WindowFlags());
    ~ManageHistoryDialog();

protected:
    enum { FilenameRole = Qt::UserRole+1, GnupgHomeRole};

public slots:
    int exec();

protected slots:
    void itemSelectionChanged();
    void deleteItem();
    void editingFilename(const QString & text);
    void editingGnupgHome(const QString & text);

private:
    struct Private;
    Private * p;
};

#endif // _MANAGEHISTORYDIALOG_H_
