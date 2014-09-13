/*
 * controlserver.cpp
 *
 * Created on: Oct 22, 2013
 * Author: Sergey Stolyarov
 */

#include <QtWidgets>
#include <QtNetwork>
#include <QtDebug>
#include <QTest>
#include <QEventLoop>

#include "controlpeer.h"
#include "encryptedtextwindow.h"
#include "gpgmewrapper.h"
#include "settings.h"

struct ControlPeer::Private
{
    QLocalServer * server;
    ControlPeer::Error error;
    ControlPeer::Mode mode;
};

ControlPeer * ControlPeer::inst = 0;

const char * ControlPeer::ACK = "ack";
const char * ControlPeer::SHOW_DIALOG_METHOD = "show-dialog-bla-bla";
const char * ControlPeer::OPEN_FILE_METHOD = "open-editor-bla-bla";
const char * ControlPeer::IS_ALIVE_METHOD = "is-alive";

ControlPeer::ControlPeer(QObject * parent)
    : QObject(parent)
{
    p = new Private;
    p->error = NoError;
    p->mode = ModeUndefined;

    QString controlSocketPath = Guzum::Config::controlSocketPath();

    p->server = new QLocalServer(this);
    bool res = p->server->listen(controlSocketPath);
    if (!res) {
        if (p->server->serverError() == QAbstractSocket::AddressInUseError) {
            // check is the server alive, if not then remove socket file and try again
            p->mode = ModeClient;
            QString response = sendRawMessage("test", 1000);
            if (error() != NoError) {
                // failed to send message so there is no listeners there
                // so try to start again
                QFile::remove(controlSocketPath);
                res = p->server->listen(controlSocketPath);
                if (!res) {
                    p->error = FailedToStartServerError;
                    p->mode = ModeUndefined;
                } else {
                    p->error = NoError;
                    p->mode = ModeServer;
                    QObject::connect(p->server, SIGNAL(newConnection()), SLOT(receiveConnection()));
                }
                return;
            } else {
                // service responded, so we are really in client mode
                p->error = NoError;
                p->mode = ModeClient;
            }
        } else {
            p->error = FailedToStartServerError;
            p->mode = ModeUndefined;
        }
    } else {
        p->error = NoError;
        p->mode = ModeClient;
    }
}

ControlPeer * ControlPeer::instance()
{
    if (inst == 0) {
        inst = new ControlPeer();
    }

    return inst;
}

void ControlPeer::receiveConnection()
{
    QLocalSocket* socket = p->server->nextPendingConnection();
    if (!socket) {
        return;
    }

    while (socket->bytesAvailable() < (int)sizeof(quint32)) {
        socket->waitForReadyRead();
    }

    QDataStream ds(socket);
    QByteArray uMsg;    
    quint32 remaining;
    ds >> remaining;
    uMsg.resize(remaining);
    int got = 0;
    char* uMsgBuf = uMsg.data();
    do {
        got = ds.readRawData(uMsgBuf, remaining);
        remaining -= got;
        uMsgBuf += got;
    } while (remaining && got >= 0 && socket->waitForReadyRead(2000));

    if (got < 0) {
        qWarning("Guzum.ControlPeer: Message reception failed %s", socket->errorString().toLatin1().constData());
        delete socket;
        return;
    }

    QString message(QString::fromUtf8(uMsg));
    socket->write(ACK, qstrlen(ACK));
    socket->waitForBytesWritten(1000);
    delete socket;

    // split message into the tokens, the format is the following:
    // <method_name>\n<arg0>\n<arg1> etc
    QStringList tokens = message.split("\n");
    QString methodName = tokens[0];

    if (methodName == SHOW_DIALOG_METHOD) {
        showFileSelectorDialog();
    } else if (methodName == OPEN_FILE_METHOD) {
        if (tokens.size() == 2) {
            // just open file using default gnupg home
            QString filename = tokens[1];
            editFile(filename);
        } else if (tokens.size() == 3) {
            // use file and custom gnupg home
            QString filename = tokens[1];
            QString gnupgHome = tokens[2];
            editFile(filename, gnupgHome);
        }
        
        QString filename = message.mid(qstrlen(OPEN_FILE_METHOD)+1);
    }
}

QString ControlPeer::sendRawMessage(const QString & msg, int timeout)
{
    if (mode() != ModeClient) {
        p->error = NotInClientModeError;
        return QString();
    }

    QString controlSocketPath = Guzum::Config::controlSocketPath();
    QLocalSocket socket;
    bool res = false;

    socket.connectToServer(controlSocketPath);
    res = socket.waitForConnected(timeout);

    if (!res) {
        p->error = ReadFailedError;
        return QString();
    }

    QByteArray bytes(msg.toUtf8());
    QByteArray responseBytes;
    QDataStream ds(&socket);
    ds.writeBytes(bytes.constData(), bytes.size());
    res = socket.waitForBytesWritten(timeout);
    if (res) {
        res &= socket.waitForReadyRead(timeout);   // wait for ack
        if (res) {
            responseBytes = socket.read(qstrlen(ACK));
            res &= (responseBytes == ACK);
        }
    }

    if (!res) {
        p->error = ReadFailedError;
        return QString();
    }
    p->error = NoError;
    return QString::fromUtf8(responseBytes.constData());
}

void ControlPeer::showFileSelectorDialog()
{
    if (mode() == ModeServer) {
        QString editFilename;
        // read last used dir name from the settings
        QSettings * settings = Guzum::Config::settings();
        settings->beginGroup("EditFileSelector");
        QString initDir = settings->value("init-dir").toString();
        settings->endGroup();

        if (initDir.isEmpty()) {
            initDir = QDir::homePath();
        }
        QPointer<QFileDialog> fileDialog = new QFileDialog(0, EncryptedTextWindow::tr("Select file encrypted by Gnupg"),
            initDir, EncryptedTextWindow::tr("Encrypted files (*.gpg, *.asc) (*.gpg *.asc);;All files (*.*)"));

        if (fileDialog->exec()) {
            QStringList files = fileDialog->selectedFiles();
            editFilename = files[0];
        }

        if (editFilename.isEmpty()) {
            return;
        }

#ifdef Q_OS_MAC
        // special workaround for MacOSX
        // to let system hide dialog
        QTest::qWait(500);
#endif
        editFile(editFilename);
        // remember last used directory
        QFileInfo fi(editFilename);
        settings->beginGroup("EditFileSelector");
        settings->setValue("init-dir", fi.canonicalPath());
        settings->endGroup();
        settings->sync();
    } else if (mode() == ModeClient) {
        sendRawMessage(SHOW_DIALOG_METHOD, 1000);
    }
}

void ControlPeer::editFile(const QString & fn, const QString & gnupgHome)
{
    QString filename(fn);

    if (mode() == ModeServer) {
        EncryptedTextWindow * textWindow;
        QFileInfo fi(filename);
        if (!fi.exists()) {
            QMessageBox::critical(0, EncryptedTextWindow::tr("Error"), 
                    EncryptedTextWindow::tr("File `%1' not found").arg(filename));
            return;
        }
        // init gpgme
        GPGME_Error err = GPGME::init(gnupgHome);
        if (err != GPG_ERR_NO_ERROR) {
            QMessageBox::critical(0, EncryptedTextWindow::tr("Error"),
                    EncryptedTextWindow::tr("Cannot initialize GPG backend"));
            return;
        }

        filename = fi.canonicalFilePath();
        textWindow = new EncryptedTextWindow(filename, gnupgHome);

        QApplication::setActiveWindow(textWindow);
        if (!textWindow->show()) {
            return;
        }
    } else if (mode() == ModeClient) {
        sendRawMessage(QString("%1\n%2\n%3").arg(OPEN_FILE_METHOD).arg(filename).arg(gnupgHome), 1000);
    }
}

void ControlPeer::editFile(const QString & fn)
{
    QString filename(fn);

    if (mode() == ModeServer) {
        EncryptedTextWindow * textWindow;
        QFileInfo fi(filename);
        if (!fi.exists()) {
            QMessageBox::critical(0, EncryptedTextWindow::tr("Error"), 
                    EncryptedTextWindow::tr("File `%1' not found").arg(filename));
            return;
        }
        // init gpgme if required
        GPGME_Error err = GPGME::init(QString());
        if (err != GPG_ERR_NO_ERROR) {
            QMessageBox::critical(0, EncryptedTextWindow::tr("Error"),
                    EncryptedTextWindow::tr("Cannot initialize GPG backend"));
            return;
        }

        filename = fi.canonicalFilePath();
        textWindow = new EncryptedTextWindow(filename, QString());

        QApplication::setActiveWindow(textWindow);
        if (!textWindow->show()) {
            return;
        }
    } else if (mode() == ModeClient) {
        sendRawMessage(QString("%1\n%2").arg(OPEN_FILE_METHOD).arg(filename), 1000);
    }
}

ControlPeer::Mode ControlPeer::mode()
{
    return p->mode;
}

ControlPeer::Error ControlPeer::error()
{
    return p->error;
}
