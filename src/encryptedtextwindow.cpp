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
#include "gpgmewrapper.h"

struct EncryptedTextWindow::Private
{
    QString filename;
    QString filenameHash;
    QPlainTextEdit * editor;
    QToolBar * topToolBar;
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

    QSettings * settings = Guzum::Config::settings();

    // create actions
    QAction * quitAction = new QAction(tr("&Quit"), this);
    QAction * aboutAction = new QAction(tr("&About Guzum"), this);

    // connect signals
    connect(quitAction, SIGNAL(triggered()),
            this, SLOT(close()));
    connect(aboutAction, SIGNAL(triggered()),
            this, SLOT(showAboutDialog()));

    // add basic control elements: top toolbar (fixed, not movable/resizeable), buttons on that toolbar, mainmenu
    QMenu * fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(quitAction);
    QMenu * helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAction);

    // create widgets
    p->editor = new QPlainTextEdit(this);
    //layout()->addWidget(p->editor);
    setCentralWidget(p->editor);
    setFocusProxy(p->editor);

    p->topToolBar = new QToolBar(this);
    p->topToolBar->setAllowedAreas(Qt::TopToolBarArea);
    p->topToolBar->setMovable(false);
    p->topToolBar->setObjectName("MainToolbar");
    addToolBar(p->topToolBar);
    
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

void EncryptedTextWindow::close()
{
    QMainWindow::close();
}

void EncryptedTextWindow::show()
{
    QMainWindow::show();

    // now try to decrypt and load data from the file
    GPGME * gpg = GPGME::instance();
    gpg->decryptFile(p->filename);
    if (gpg->error() != GPG_ERR_NO_ERROR) {
        // failed to decrypt file
        switch (gpg->error()) {
        case GPG_ERR_INV_VALUE:
            qDebug() << "GPG_ERR_INV_VALUE";
            break;

        case GPG_ERR_NO_DATA:
            qDebug() << "GPG_ERR_NO_DATA";
            break;

        case GPG_ERR_DECRYPT_FAILED:
            qDebug() << "GPG_ERR_DECRYPT_FAILED";
            break;

        case GPG_ERR_BAD_PASSPHRASE:
            qDebug() << "GPG_ERR_BAD_PASSPHRASE";
            break;

        case GPG_ERR_CANCELED:
            qDebug() << "passphrase input has been canceled";
            break;

        default:
            qDebug() << "Other decryption error: " << gpg->error();
            qDebug() << "GPG_ERR_SYSTEM_ERROR" << GPG_ERR_SYSTEM_ERROR;
        }
    }
}

void EncryptedTextWindow::showAboutDialog()
{
    qDebug() << "showAboutDialog()";
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


