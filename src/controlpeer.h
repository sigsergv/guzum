/*
 * controlserver.h
 *
 * Created on: Oct 22, 2013
 * Author: Sergey Stolyarov
 */

#ifndef _CONTROLPEER_H_
#define _CONTROLPEER_H_

#include <QObject>

class ControlPeer : public QObject
{
    Q_OBJECT

public:
    enum Error { NoError, FailedToStartServerError, NotInClientModeError, ReadFailedError };
    enum Mode { ModeUndefined, ModeClient, ModeServer };
    static ControlPeer * instance();

    Error error();
    Mode mode();

    void showFileSelectorDialog();
    void editFile(const QString & filename);

protected:
    static const char * ACK;
    static const char * SHOW_DIALOG_METHOD;
    static const char * OPEN_FILE_METHOD;
    QString sendRawMessage(const QString & msg, int timeout);

protected slots:
    void receiveConnection();

private:
    ControlPeer(QObject * parent = 0);
    static ControlPeer * inst;
    struct Private;
    Private * p;
};

#endif // _CONTROLPEER_H_