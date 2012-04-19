/*
 * encryptedtextwindow.h
 *
 * Created on: Apr 19, 2012
 * Author: Sergei Stolyarov
 */

#ifndef _ENCRYPTEDTEXTWINDOW_H_
#define _ENCRYPTEDTEXTWINDOW_H_

#include <QMainWindow>

struct QString;

class EncryptedTextWindow : public QMainWindow
{
public:
    EncryptedTextWindow(const QString & filename, QWidget * parent = 0);
    ~EncryptedTextWindow();

private:
    struct Private;
    Private * p;
};

#endif // _ENCRYPTEDTEXTWINDOW_H_
