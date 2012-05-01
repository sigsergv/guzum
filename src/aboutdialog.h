/*
 * aboutdialog.h
 *
 * Created on: May 01, 2012
 * Author: Sergei Stolyarov
 */

#ifndef _ABOUTDIALOG_H_
#define _ABOUTDIALOG_H_

#include <QDialog>

class AboutDialog : public QDialog
{
    Q_OBJECT
public:
    AboutDialog(QWidget * parent = 0, Qt::WindowFlags f = 0);
    ~AboutDialog();

private:
    struct Private;
    Private * p;
};

#endif // _ABOUTDIALOG_H_
