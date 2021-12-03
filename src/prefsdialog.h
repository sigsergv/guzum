/*
 * prefsdialog.h
 *
 * Created on: May 01, 2012
 * Author: Sergey Stolyarov
 */

#ifndef _PREFSDIALOG_H_
#define _PREFSDIALOG_H_

#include <QDialog>

class PrefsDialog : public QDialog
{
    Q_OBJECT
public:
    PrefsDialog(QWidget * parent = 0, Qt::WindowFlags f = Qt::WindowFlags());
    ~PrefsDialog();

public slots:
    int exec();

protected slots:
    void setAutostartToggle(int state);
    void uiLanguageChanged(int index);

private:
    struct Private;
    Private * p;
};

#endif // _PREFSDIALOG_H_
