/*
 * gpgmewrapper.h
 *
 * Created on: Apr 20, 2012
 * Author: Sergei Stolyarov
 */

#ifndef _GPGMEWRAPPER_H_
#define _GPGMEWRAPPER_H_
#include <gpgme.h>

typedef unsigned int GPGME_Error;

typedef enum {
    GPGME_WRAPPER_ERR_BASE = GPG_ERR_CODE_DIM+1,
    GPGME_WRAPPER_ERR_FILE_NOT_FOUND,
    GPGME_WRAPPER_ERR_CANNOT_OPEN_FILE,
    GPGME_WRAPPER_ERR_CANNOT_FIND_KEY,
    GPGME_WRAPPER_ERR_FILE_TOO_LARGE,
    GPGME_WRAPPER_ERR_DATA_TOO_LARGE,

    GPGME_WRAPPER_ERR_DIM // this code must be last in the list

} GPGME_Wrapper_Errors;

struct QString;
struct QWidget;

class GPGME {
public:
    static GPGME * instance();
    static GPGME_Error init();
    GPGME_Error error();


    QByteArray decryptFile(const QString & filename, QString & uid, QWidget * parent = 0);
    void encryptBytesToFile(const QByteArray & data, const QString & filename, const QString & uid);
    
private:
    GPGME(gpgme_ctx_t context);
    ~GPGME();
    static GPGME * inst;
    void setError(GPGME_Error error, bool wrapperError = false);

    struct Private;
    Private * p;
};

#endif // _GPGMEWRAPPER_H_
