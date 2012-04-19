/*
 * encryptedtextwindow.cpp
 *
 * Created on: Apr 19, 2012
 * Author: Sergei Stolyarov
 */

#include <QtDebug>

#include "encryptedtextwindow.h"

struct EncryptedTextWindow::Private
{
    int f;
};


EncryptedTextWindow::EncryptedTextWindow(const QString & filename, QWidget * parent)
    : QMainWindow(parent)
{
    p = new Private;
}

EncryptedTextWindow::~EncryptedTextWindow()
{
    delete p;
}
