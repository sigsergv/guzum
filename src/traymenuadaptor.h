/*
 * traymenuadaptor.h
 *
 * Created on: Apr 29, 2012
 * Author: Sergei Stolyarov
 */

#ifndef _TRAYMENUADAPTOR_H_
#define _TRAYMENUADAPTOR_H_

#include <QDBusAbstractAdaptor>

struct QString;

class TrayMenuAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.regolit.guzum.tray")
    /*
    Q_CLASSINFO("D-Bus Introspection", ""
        "<interface name=\"com.regolit.guzum.tray\">\n"
        "  <method name=\"appendFile\"/>\n"
        "</interface>")
    */

public:
    TrayMenuAdaptor(QObject * parent);
    virtual ~TrayMenuAdaptor();

public slots:
    void appendFile(const QString & filename);
};

#endif // _TRAYMENUADAPTOR_H_
