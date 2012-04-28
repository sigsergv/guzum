/*
 * gpgmewrapper.cpp
 *
 * Created on: Apr 20, 2012
 * Author: Sergei Stolyarov
 */

#include <QtDebug>
#include <QDir>
#include <QInputDialog>
#include <locale.h>
#include <unistd.h>

#include "gpgmewrapper.h"
#include "passphrasedialog.h"

gpgme_ctx_t context;

struct CallbackData {
    QWidget * parent;
};

struct GPGME::Private
{
    gpgme_ctx_t context;
    GPGME_Error error;
};

GPGME::GPGME(gpgme_ctx_t context)
{
    p = new Private;
    p->context = context;
    p->error = GPG_ERR_NO_ERROR;
}

GPGME::~GPGME()
{
    delete p;
}

GPGME * GPGME::inst = 0;

// passphrase callback
gpgme_error_t passphraseCallback(void * hook, const char * uid_hint, 
        const char * passphrase_info, int prev_was_bad, int fd)
{
    Q_UNUSED(passphrase_info);

    CallbackData * cbData = static_cast<CallbackData *>(hook);
    PassphraseDialog passDlg(cbData->parent);
    passDlg.setUid(uid_hint);
    if (prev_was_bad) {
        passDlg.triggerErrorLabel();
    }
    if (passDlg.exec() == QDialog::Rejected) {
        return GPG_ERR_CANCELED;
    }

    QString pass = passDlg.passphrase() + "\n";
    QByteArray passBytes = pass.toAscii();
    write(fd, passBytes.constData(), passBytes.size());
    return GPG_ERR_NO_ERROR;
}

GPGME_Error GPGME::init()
{
    setlocale(LC_ALL, "");
    gpgme_check_version(NULL);
    gpgme_set_locale(NULL, LC_CTYPE, setlocale (LC_CTYPE, NULL));
#ifdef LC_MESSAGES
    gpgme_set_locale (NULL, LC_MESSAGES, setlocale (LC_MESSAGES, NULL));
#endif

    gpgme_error_t err;
    gpgme_ctx_t context;

    err = gpgme_engine_check_version(GPGME_PROTOCOL_OpenPGP);
    if (err != GPG_ERR_NO_ERROR) {
        return err;
    }

    QString protocolName =  gpgme_get_protocol_name(GPGME_PROTOCOL_OpenPGP);
    qDebug() << "protocol: " << protocolName;

    gpgme_engine_info_t engineInfo;
    err = gpgme_get_engine_info(&engineInfo);
    if (err != GPG_ERR_NO_ERROR) {
        return err;
    }
    qDebug() << "Backend info";
    qDebug() << "filename: " << engineInfo->file_name << ", homedir: " << engineInfo->home_dir;

    err = gpgme_new(&context);
    if (err != GPG_ERR_NO_ERROR) {
        return err;
    }

    err = gpgme_set_protocol(context, GPGME_PROTOCOL_OpenPGP);
    if (err != GPG_ERR_NO_ERROR) {
        return err;
    }

    // we don't need to set gnupg home dir explicitly
    err = gpgme_ctx_set_engine_info(context, GPGME_PROTOCOL_OpenPGP, 
            engineInfo->file_name,
            engineInfo->home_dir
            );
    if (err != GPG_ERR_NO_ERROR) {
        return err;
    }

    inst = new GPGME(context);
    qDebug() << "gpgme initalized";

    return GPG_ERR_NO_ERROR;
}

GPGME_Error GPGME::error()
{
    return gpg_err_code(p->error);
}

void GPGME::setError(GPGME_Error error)
{
    p->error = error;
}


QByteArray GPGME::decryptFile(const QString & filename, QString & uid, QWidget * parent)
{
    setError(GPG_ERR_NO_ERROR); // clear error
    CallbackData cbData;
    cbData.parent = parent;

    gpgme_set_passphrase_cb(p->context, passphraseCallback, &cbData);

    qDebug() << "decrypting file" << filename;
    gpgme_data_t data;
    gpgme_error_t err;

    QFile file(filename);
    if (!file.exists()) {
        setError(GPGME_WRAPPER_ERR_FILE_NOT_FOUND);
        return QByteArray();
    }
    if (!file.open(QIODevice::ReadOnly)) {
        setError(GPGME_WRAPPER_ERR_CANNOT_OPEN_FILE);
        return QByteArray();
    }

    err = gpgme_data_new_from_fd(&data, file.handle());
    if (err != GPG_ERR_NO_ERROR) {
        setError(err);
        return QByteArray();
    }

    // process data (it's cipher)
    gpgme_data_t plain;
    gpgme_data_new(&plain);
    err = gpgme_op_decrypt(p->context, data, plain);
    // unset passphrase callback
    gpgme_set_passphrase_cb(p->context, 0, 0);
    if (err != GPG_ERR_NO_ERROR) {
        gpgme_data_release(data);
        gpgme_data_release(plain);
        qDebug() << "aaaaaaaaa";
        setError(err);
        return QByteArray();
    }

    // decryption is successful so load plain data
    QByteArray resBytes;
    const int bufSize = 1000;
    int read;
    char buf[bufSize];

    gpgme_data_rewind(plain);
    while (true) {
        read = gpgme_data_read(plain, (void*)buf, bufSize);
        if (read == 0) {
            break;
        }
        if (read == -1) {
            // error, ignore for now
            break;
        }
        resBytes.append(buf, read);
    }

    gpgme_decrypt_result_t decrypt_res = gpgme_op_decrypt_result(p->context);
    gpgme_recipient_t recipient;
    if (decrypt_res) {
        recipient = decrypt_res->recipients;
        while (recipient) {
            uid = recipient->keyid;
            recipient = recipient->next;
            break; // just ignore the other recipients
        }
    }
    return resBytes;
}

void GPGME::encryptBytesToFile(const QByteArray & data, const QString & filename, const QString & uid)
{
    setError(GPG_ERR_NO_ERROR); // clear error
    qDebug() << "Encrypt data to file" << filename;
    gpgme_error_t err;

    // list all available keys and find the appropriate
    err = gpgme_op_keylist_start(p->context, uid.toAscii().data(), 0);
    gpgme_key_t key = 0;

    while (1) {
        err = gpgme_op_keylist_next(p->context, &key);
        if (err == GPG_ERR_NO_ERROR) {
            // key found
            qDebug() << "KEY FOUND";
        }
        break; // take just one key
    }
    gpgme_op_keylist_end(p->context);

    // if filename ends with ".asc" then use armored output, otherwise use binary

    gpgme_data_t cipher;
    
    return;
    // prepare file for writing
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        setError(GPGME_WRAPPER_ERR_CANNOT_OPEN_FILE);
        return;
    }

    err = gpgme_data_new_from_fd(&cipher, file.handle());
    if (err != GPG_ERR_NO_ERROR) {
        setError(err);
        return;
    }

    gpgme_data_t plain;
    gpgme_data_new_from_mem(&plain, data.data(), data.length(), 0); // do not copy data

    //err = gpgme_op_encrypt(p->context, );

}

GPGME * GPGME::instance()
{
    return inst;
}

