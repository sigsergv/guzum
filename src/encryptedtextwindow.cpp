/*
 * encryptedtextwindow.cpp
 *
 * Created on: Apr 19, 2012
 * Author: Sergey Stolyarov
 */

#include <QtCore>
#include <QtWidgets>
#include <QtDebug>

#include "encryptedtextwindow.h"
#include "settings.h"
#include "macos.h"
#include "gpgmewrapper.h"
#include "traymanager.h"
#include "aboutdialog.h"
#include "secureplaintexteditor.h"

struct EncryptedTextWindow::Private
{
    QString filename;
    QString filenameHash;
    QString windowTitleBase;
    SecurePlainTextEditor * editor;
    QToolBar * topToolBar;
    QString gpgUid; // UID for encrypting/decrypting
    QAction * saveAction;
    QLabel * timerLabel;
    QTimer * autoCloseTimer;
    unsigned int autoCloseCounter;
    QString gnupgHome;
};

static const QString RND_CHARACTERS_SET = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
static unsigned int AUTO_CLOSE_TIMEOUT = 600; // in seconds

EncryptedTextWindow::EncryptedTextWindow(const QString & filename, const QString & gnupgHome, QWidget * parent)
    : QMainWindow(parent)
{
    p = new Private;
    QFileInfo fi(filename);
    p->filename = fi.canonicalFilePath();
    p->gnupgHome = gnupgHome;

    p->autoCloseTimer = new QTimer(this);
    p->autoCloseTimer->setInterval(1000);
    p->autoCloseTimer->setSingleShot(false);

    p->autoCloseCounter = AUTO_CLOSE_TIMEOUT;

    p->windowTitleBase = QString("%1 [guzum]").arg(fi.fileName());
    setWindowTitle(p->windowTitleBase);

    // compute filename hashsum, it's used for storing settings for example
    QCryptographicHash h(QCryptographicHash::Sha1);
    QByteArray utfData = p->filename.toUtf8();
    h.addData(utfData.constData(), utfData.length());
    p->filenameHash = h.result().toHex();

    QSettings * settings = Guzum::Config::settings();

    // create actions
    QAction * quitAction = new QAction(tr("&Quit"), this);
    QAction * aboutAction = new QAction(tr("&About Guzum"), this);
    QAction * saveAction = new QAction(QIcon(":/save-22x22.png"), tr("&Save file"), this);
    QAction * changeCurrentFontAction = new QAction(QIcon(":/text-22x22.png"), tr("Change current font"), this);
    QAction * changeDefaultFontAction = new QAction(tr("Set default &font"), this);
    QAction * insertRandomStringAction = new QAction(QIcon(":/insert-text-22x22.png"), tr("Insert random string in the caret position"), this);

    saveAction->setShortcut(QKeySequence(Qt::Key_S + Qt::CTRL));
    saveAction->setDisabled(true);

    // create widgets
    p->timerLabel = new QLabel("00:00");
    QFont font = p->timerLabel->font();
    font.setBold(true);
    p->timerLabel->setFont(font);
    p->timerLabel->setToolTip(tr("Time remaining to automatical window close"));

    p->editor = new SecurePlainTextEditor(this);
    //layout()->addWidget(p->editor);
    setCentralWidget(p->editor);
    setFocusProxy(p->editor);

    // connect signals
    connect(quitAction, SIGNAL(triggered()),
            this, SLOT(close()));
    connect(aboutAction, SIGNAL(triggered()),
            this, SLOT(showAboutDialog()));
    connect(saveAction, SIGNAL(triggered()),
            this, SLOT(saveFile()));
    connect(changeCurrentFontAction, SIGNAL(triggered()),
            this, SLOT(changeCurrentFont()));
    connect(changeDefaultFontAction, SIGNAL(triggered()),
            this, SLOT(changeDefaultFont()));
    connect(p->editor, SIGNAL(modificationChanged(bool)),
            this, SLOT(editorModificationChanged(bool)));
    connect(insertRandomStringAction, SIGNAL(triggered()),
            this, SLOT(insertRandomString()));
    connect(p->autoCloseTimer, SIGNAL(timeout()),
            this, SLOT(closeTimerTick()));

    // add basic control elements: top toolbar (fixed, not movable/resizeable), buttons on that toolbar, mainmenu
    QMenu * fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(saveAction);
    fileMenu->addAction(changeDefaultFontAction);
    fileMenu->addAction(quitAction);
    QMenu * helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAction);

    p->topToolBar = new QToolBar(this);
    p->topToolBar->setAllowedAreas(Qt::TopToolBarArea);
    p->topToolBar->setMovable(false);
    p->topToolBar->setObjectName("MainToolbar");
    // add actions to the toolbar
    p->topToolBar->addAction(saveAction);
    p->topToolBar->addAction(changeCurrentFontAction);
    p->topToolBar->addSeparator();
    p->topToolBar->addAction(insertRandomStringAction);
    p->topToolBar->addSeparator();
    p->topToolBar->addWidget(p->timerLabel);
    addToolBar(p->topToolBar);
    
    // try to restore window settings
    settings->beginGroup("Windows");
    QString key;
    QByteArray bv;

    key = QString("%1-geometry").arg(p->filenameHash);
    bv = settings->value(key).toByteArray();
    if (!bv.isEmpty()) {
        restoreGeometry(bv);
    } else {
        // set default width and height
        resize(600, 400);
    }

    key = QString("%1-state").arg(p->filenameHash);
    restoreState(settings->value(key).toByteArray());
    key = QString("%1-font").arg(p->filenameHash);
    QVariant value = settings->value(key);
    settings->endGroup();
    if (value.canConvert<QFont>()) {
        p->editor->setFont(value.value<QFont>());
    } else {
        settings->beginGroup("Defaults");
        value = settings->value("font");
        settings->endGroup();
        if (value.canConvert<QFont>()) {
            p->editor->setFont(value.value<QFont>());
        } else {
            // use "fallback" monospace font
            QFont font("Monospace");
            font.setStyleHint(QFont::TypeWriter);
            p->editor->setFont(font);
        }
    }

    p->saveAction = saveAction;

    p->editor->installEventFilter(this);
    
#ifdef Q_OS_MAC
    // show app icon in osx dock
    setDockIconStyle(false);
#endif
}

EncryptedTextWindow::~EncryptedTextWindow()
{
    delete p;
}

void EncryptedTextWindow::close()
{
    QMainWindow::close();
}

bool EncryptedTextWindow::show()
{
    // now try to decrypt and load data from the file
    GPGME * gpg = GPGME::instance(p->gnupgHome);
    QByteArray decrypted = gpg->decryptFile(p->filename, p->gpgUid, this);
    if (gpg->error() != GPG_ERR_NO_ERROR) {
        // failed to decrypt file
        QString errorMessage;
        switch (gpg->error()) {
        case GPG_ERR_INV_VALUE:
            qDebug() << "GPG_ERR_INV_VALUE";
            errorMessage = tr("GPG_ERR_INV_VALUE");
            break;

        case GPG_ERR_NO_DATA:
            qDebug() << "GPG_ERR_NO_DATA";
            errorMessage = tr("GPG_ERR_NO_DATA");
            break;

        case GPGME_WRAPPER_ERR_FILE_TOO_LARGE:
            qDebug() << "GPGME_WRAPPER_ERR_FILE_TOO_LARGE";
            errorMessage = tr("File is too large to decrypt.");
            break;

        case GPG_ERR_DECRYPT_FAILED:
            qDebug() << "GPG_ERR_DECRYPT_FAILED";
            errorMessage = tr("Unable to decrypt message, most probably private key for the encrypted file has not been found.");
            break;

        case GPG_ERR_BAD_PASSPHRASE:
            qDebug() << "GPG_ERR_BAD_PASSPHRASE";
            // not passphrase, most probably user closed password dialog or attempts limit is reached
            // errorMessage = tr("Incorrect passphrase.");
            break;

        case GPG_ERR_CANCELED:
            qDebug() << "passphrase input has been canceled";
            break;

        default:
            qDebug() << "Other decryption error: " << gpg->error();
            qDebug() << "GPG_ERR_SYSTEM_ERROR" << GPG_ERR_SYSTEM_ERROR;
        }
        if (!errorMessage.isEmpty()) {
            QMessageBox::critical(0, tr("Decryption failed"), errorMessage);
        }
        return false;
    } else {
        // load data into the editor
        // TODO: guess encoding? Assume it's UTF-8 for now
        QString contents = QString::fromUtf8(decrypted.constData());
        p->editor->setPlainText(contents);
        QMainWindow::show();

        TrayManager * tm = TrayManager::instance();
        tm->appendFile(p->filename, gpg->gpgHomeDir());

        // bring window to front
        raise();
        activateWindow();
        return true;
    }
}

void EncryptedTextWindow::showAboutDialog()
{
    AboutDialog dlg(this);
    dlg.exec();
}

void EncryptedTextWindow::saveFile()
{
    // write data back to file, i.e. encrypt them
    qDebug() << "encrypt data using key " << p->gpgUid;
    if (p->gpgUid.isEmpty()) {
        return;
    }
    GPGME * gpg = GPGME::instance(p->gnupgHome);
    QByteArray data = p->editor->toPlainText().toUtf8();
    gpg->encryptBytesToFile(data, p->filename, p->gpgUid);
    if (gpg->error() != GPG_ERR_NO_ERROR) {
        QString errorMessage;
        switch (gpg->error()) {
        case GPGME_WRAPPER_ERR_DATA_TOO_LARGE:
            errorMessage = tr("Data too large to encrypt.");
            break;

        case GPGME_WRAPPER_ERR_CANNOT_FIND_KEY:
            errorMessage = tr("Cannot find public key to encrypt data.");
            break;

        case GPGME_WRAPPER_ERR_CANNOT_OPEN_FILE:
            errorMessage = tr("Cannot open target file.");
            break;

        case GPGME_WRAPPER_ERR_FILE_TOO_LARGE:
            errorMessage = tr("Cannot backup encrypted file, it's too large.");
            break;

        case GPGME_WRAPPER_ERR_MORE_THAN_ONE_KEY_FOUND:
            errorMessage = tr("More than one key found for key ID “%1”").arg(p->gpgUid);
            break;

        case GPG_ERR_UNUSABLE_PUBKEY:
            errorMessage = tr("Unusable public key (maybe expired or revoked)");
            break;

        case GPG_ERR_INV_VALUE:
            errorMessage = tr("GPG_ERR_INV_VALUE");
            break;

        default:
            errorMessage = tr("Unknown error: %1").arg(gpg->error());
        }
        if (!errorMessage.isEmpty()) {
            QMessageBox::critical(0, tr("Encryption failed"), errorMessage);
        }
    } else {
        // file is successfully written so change editor modification state
        p->editor->document()->setModified(false);
    }
}

void EncryptedTextWindow::changeCurrentFont()
{
    bool ok;
    QFont currentFont = p->editor->font();
    QFont font = QFontDialog::getFont(&ok, currentFont, this);
    // change window font, also remember it in the settings
    if (ok) {
        p->editor->setFont(font);
        QSettings * settings = Guzum::Config::settings();
        settings->beginGroup("Windows");
        QString key = QString("%1-font").arg(p->filenameHash);
        settings->setValue(key, font);
        settings->endGroup();
    }
}

void EncryptedTextWindow::changeDefaultFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, this);

    if (ok) {
        QSettings * settings = Guzum::Config::settings();
        settings->beginGroup("Defaults");
        settings->setValue("font", font);
        settings->endGroup();
    }
}

void EncryptedTextWindow::insertRandomString()
{
    QString rnd;
    int len = RND_CHARACTERS_SET.length();

    for (int i=0; i<10; i++) {
        rnd += RND_CHARACTERS_SET[qrand() % len];
    }
    p->editor->insertPlainText(rnd);
}

void EncryptedTextWindow::closeTimerTick()
{
    p->autoCloseCounter--;

    if (p->autoCloseCounter <= 0) {
        p->autoCloseTimer->stop();
        close();
    } else {
        // update counter
        unsigned int s = p->autoCloseCounter;
        QString time;
        unsigned int seconds = s % 60;
        s /= 60;
        unsigned int minutes = s % 60;
        s /= 60;
        unsigned int hours = s;

        time = QString("%1").arg(seconds, 2, 10, QLatin1Char('0'));
        time = QString("%1:").arg(minutes, 2, 10, QLatin1Char('0')) + time;
        if (hours > 0) {
            time = QString("%1:").arg(hours, 2, 10, QLatin1Char('0')) + time;
        }

        p->timerLabel->setText(time);

        // also update window title
        setWindowTitle(QString("[%1] %2").arg(time).arg(p->windowTitleBase));
    }
}

void EncryptedTextWindow::editorModificationChanged(bool changed)
{
    if (changed) {
        p->saveAction->setDisabled(false);
    } else {
        p->saveAction->setDisabled(true);
    }
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


bool EncryptedTextWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == p->editor) {
        if (event->type() == QEvent::FocusOut) {
            p->autoCloseCounter = AUTO_CLOSE_TIMEOUT;
            p->autoCloseTimer->start();
        } else if (event->type() == QEvent::FocusIn) {
            // stop timer
            p->autoCloseTimer->stop();
            p->autoCloseCounter = AUTO_CLOSE_TIMEOUT + 1;
            closeTimerTick();
            setWindowTitle(p->windowTitleBase);
        }
        
    }
    return QObject::eventFilter(obj, event);
}
