/*
 * traymenuadaptor.cpp
 *
 * Created on: Apr 29, 2012
 * Author: Sergei Stolyarov
 */

#include <QtDebug>
#include <QMetaObject>

#include "traymenuadaptor.h"

TrayMenuAdaptor::TrayMenuAdaptor(QObject * parent)
    : QDBusAbstractAdaptor(parent)
{
}

TrayMenuAdaptor::~TrayMenuAdaptor()
{
}

void TrayMenuAdaptor::appendFile(const QString & filename, const QString & gnupgHome)
{
    QMetaObject::invokeMethod(parent(), "appendFile", Q_ARG(QString, filename), Q_ARG(QString, gnupgHome));
}
