/*
 * gpgmewrapper.cpp
 *
 * Created on: Apr 20, 2012
 * Author: Sergey Stolyarov
 */

#include <QtDebug>
#include <QDir>
#include <QInputDialog>
#include <locale.h>
#include <unistd.h>
#include <stdlib.h>

#include "gpgmewrapper.h"
#include "passphrasedialog.h"

// 100 MB limit
#define FILESIZE_HARD_LIMIT 104857610

gpgme_ctx_t context;

struct CallbackData {
    QWidget * parent;
};

struct GPGME::Private
{
    gpgme_ctx_t context;
    GPGME_Error error;
    QString gpgHomeDir;
};

GPGME::GPGME(gpgme_ctx_t context, const QString & gpgHomeDir)
{
    p = new Private;
    p->context = context;
    p->error = GPG_ERR_NO_ERROR;
    p->gpgHomeDir = gpgHomeDir;
}

GPGME::~GPGME()
{
    delete p;
}

// contains instances of GPGME for each GNUPGHOME value
static QMap<QString, GPGME*> instancesStore;

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

GPGME_Error GPGME::init(const QString & gpgHomeDir)
{
    if (instancesStore.contains(gpgHomeDir)) {
        return GPG_ERR_NO_ERROR;
    }

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
    QString gnupgHome;
    if (gpgHomeDir.isEmpty()) {
        // i.e. use default gnupg directory or one from environment
        QString gnupgHomeEnv = QString::fromAscii(qgetenv("GNUPGHOME"));
        if (!gnupgHomeEnv.isEmpty()) {
            gnupgHome = gnupgHomeEnv;
        } else {
            // use default path: "~/.gnupg"
            QDir gh = QDir::home();
            gh.cd(".gnupg");
            gnupgHome = gh.canonicalPath();
        }
    } else {
        QDir gh(gpgHomeDir);
        gnupgHome = gh.canonicalPath();
    }
    qDebug() << "GNUPGHOME" << gnupgHome;
    
    err = gpgme_ctx_set_engine_info(context, GPGME_PROTOCOL_OpenPGP, 
            engineInfo->file_name,
            gnupgHome.toAscii().data()
            );
    if (err != GPG_ERR_NO_ERROR) {
        return err;
    }

    GPGME * inst = new GPGME(context, gnupgHome);
    instancesStore[gpgHomeDir] = inst;
    qDebug() << "gpgme initalized for the directory " << gnupgHome << "[store key: " << gpgHomeDir << "]";

    return GPG_ERR_NO_ERROR;
}

GPGME_Error GPGME::error()
{
    return p->error;
}

void GPGME::setError(GPGME_Error error, bool wrapperError)
{
    if (wrapperError) {
        p->error = error;
    } else {
        p->error = gpg_err_code(error);
    }
}


QByteArray GPGME::decryptFile(const QString & filename, QString & keyId, QWidget * parent)
{
    setError(GPG_ERR_NO_ERROR); // clear error
    CallbackData cbData;
    cbData.parent = parent;


    qDebug() << "ERERR" << gpg_err_source(GPG_ERR_CODE_DIM+1);

    gpgme_set_passphrase_cb(p->context, passphraseCallback, &cbData);

    qDebug() << "decrypting file" << filename;
    gpgme_data_t data;
    gpgme_error_t err;

    QFile file(filename);
    if (!file.exists()) {
        setError(GPGME_WRAPPER_ERR_FILE_NOT_FOUND, true);
        return QByteArray();
    }
    if (!file.open(QIODevice::ReadOnly)) {
        setError(GPGME_WRAPPER_ERR_CANNOT_OPEN_FILE, true);
        return QByteArray();
    }
    if (file.size() > FILESIZE_HARD_LIMIT) {
        setError(GPGME_WRAPPER_ERR_FILE_TOO_LARGE, true);
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
        setError(err);
        return QByteArray();
    }

    // decryption is successful so load plain data
    QByteArray resBytes;
    const int bufSize = 1000;
    int read;
    char buf[bufSize];

    gpgme_data_seek(plain, 0, SEEK_SET);
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
            keyId = recipient->keyid;
            recipient = recipient->next;
            break; // just ignore the other recipients
        }
    }
    return resBytes;
}

void GPGME::encryptBytesToFile(const QByteArray & data, const QString & filename, const QString & keyId)
{
    setError(GPG_ERR_NO_ERROR); // clear error
    qDebug() << "Encrypt data to file" << filename;
    gpgme_error_t err;

    // check data size
    if (data.size() > FILESIZE_HARD_LIMIT) {
        setError(GPGME_WRAPPER_ERR_DATA_TOO_LARGE, true);
        return;
    }

    // list all available keys and find the appropriate
    err = gpgme_op_keylist_start(p->context, keyId.toAscii().data(), 0);
    gpgme_key_t key = 0;
    gpgme_key_t loop_key = 0;
    int foundCount = 0;

    while (1) {
        err = gpgme_op_keylist_next(p->context, &loop_key);
        if (err != GPG_ERR_NO_ERROR) {
            break;
        }
        qDebug() << "KEY FOUND";
        if (loop_key != 0) {
            key = loop_key;
        }
        foundCount++;
    }
    gpgme_op_keylist_end(p->context);

    if (foundCount > 1) {
        setError(GPGME_WRAPPER_ERR_MORE_THAN_ONE_KEY_FOUND);
        return;
    }

    if (key == 0) {
        // key not found
        setError(GPGME_WRAPPER_ERR_CANNOT_FIND_KEY, true);
        return;
    }

    // if filename ends with ".asc" then use armored output, otherwise use binary
    if (filename.toLower().endsWith(".asc")) {
        // use armored output
        gpgme_set_armor(p->context, 1);
        qDebug() << "Encode: use armored output";
    }

    gpgme_data_t cipher;
    gpgme_data_t plain;

    gpgme_data_new_from_mem(&plain, data.data(), data.size(), 0); // do not copy data
    
    // backup file contents
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        setError(GPGME_WRAPPER_ERR_CANNOT_OPEN_FILE, true);
        return;
    }
    if (file.size() > FILESIZE_HARD_LIMIT) {
        setError(GPGME_WRAPPER_ERR_FILE_TOO_LARGE, true);
        return;
    }
    QByteArray cipherBackup = file.readAll();
    file.close();
    
    // prepare file for writing
    if (!file.open(QIODevice::WriteOnly)) {
        setError(GPGME_WRAPPER_ERR_CANNOT_OPEN_FILE, true);
        return;
    }

    err = gpgme_data_new_from_fd(&cipher, file.handle());
    if (err != GPG_ERR_NO_ERROR) {
        setError(err);
        return;
    }

    gpgme_key_t keys[2];
    keys[0] = key;
    keys[1] = 0;

    err = gpgme_op_encrypt(p->context, keys, static_cast<gpgme_encrypt_flags_t>(GPGME_ENCRYPT_ALWAYS_TRUST), plain, cipher);
    if (err != GPG_ERR_NO_ERROR) {
        // revert file contents in case of error
        file.resize(0);
        file.write(cipherBackup);
        setError(err);
    }
    file.close();
}


QString GPGME::gpgHomeDir()
{
    return p->gpgHomeDir;   
}

GPGME * GPGME::instance(const QString & gpgHomeDir)
{
    if (instancesStore.contains(gpgHomeDir)) {
        return instancesStore[gpgHomeDir];
    }

    return 0;
}

