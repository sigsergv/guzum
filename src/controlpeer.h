/*
 * controlserver.h
 *
 * Created on: Oct 22, 2013
 * Author: Sergei Stolyarov
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

    ControlPeer(QObject * parent = 0);
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
    struct Private;
    Private * p;
};

#endif // _CONTROLPEER_H_