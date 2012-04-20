/*
 * encryptedtextwindow.cpp
 *
 * Created on: Apr 19, 2012
 * Author: Sergei Stolyarov
 */

#include <QtCore>
#include <QtGui>
#include <QtDebug>

#include "encryptedtextwindow.h"
#include "settings.h"

struct EncryptedTextWindow::Private
{
    QString filename;
    QString filenameHash;
};


EncryptedTextWindow::EncryptedTextWindow(const QString & filename, QWidget * parent)
    : QMainWindow(parent)
{
    p = new Private;
    QFileInfo fi(filename);
    p->filename = fi.canonicalFilePath();

    // compute filename hashsum, it's used for storing settings for example
    QCryptographicHash h(QCryptographicHash::Sha1);
    QByteArray utfData = p->filename.toUtf8();
    h.addData(utfData.constData(), utfData.length());
    p->filenameHash = h.result().toHex();

    qDebug() << p->filenameHash;

    QSettings * settings = Guzum::Config::settings();
    
    // try to restore window settings
    settings->beginGroup("Windows");
    QString key;
    key = QString("%1-geometry").arg(p->filenameHash);
    restoreGeometry(settings->value(key).toByteArray());
    key = QString("%1-state").arg(p->filenameHash);
    restoreState(settings->value(key).toByteArray());
    settings->endGroup();
    
}

EncryptedTextWindow::~EncryptedTextWindow()
{
    delete p;
}

void EncryptedTextWindow::rememberGeometryAndState()
{
    QSettings * settings = Guzum::Config::settings();
    QString key;
    settings->beginGroup("Windows");
    key = QString("%1-geometry").arg(p->filenameHash);
    settings->setValue(key, saveGeometry());
    key = QString("%1-state").arg(p->filenameHash);
    settings->setValue(key, saveState());
    key = QString("%1-timestamp").arg(p->filenameHash);
    QDateTime dt = QDateTime::currentDateTime();
    settings->setValue(key, dt.toTime_t());
    settings->endGroup();
}

void EncryptedTextWindow::moveEvent(QMoveEvent * event)
{
    rememberGeometryAndState();
    event->accept();
}

void EncryptedTextWindow::resizeEvent(QResizeEvent * event)
{
    rememberGeometryAndState();
    event->accept();
}

void EncryptedTextWindow::closeEvent(QCloseEvent *event)
{
    QSettings * settings = Guzum::Config::settings();

    // write layout settings and sync
    rememberGeometryAndState();
    settings->sync();
    event->accept();
}


