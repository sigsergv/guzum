/*
 * passphrasedialog.h
 *
 * Created on: Apr 25, 2012
 * Author: Sergey Stolyarov
 */

#ifndef _PASSPHRASEDIALOG_H_
#define _PASSPHRASEDIALOG_H_
#include <QDialog>
#include <QString>

class PassphraseDialog : public QDialog
{
    Q_OBJECT
public:
    PassphraseDialog(QWidget * parent = 0, Qt::WindowFlags f = 0);
    ~PassphraseDialog();
    QString passphrase();
    void setUid(const QString & uid);
    void triggerErrorLabel();

public slots:

private:
    struct Private;
    Private * p;
};

#endif // _PASSPHRASEDIALOG_H_
