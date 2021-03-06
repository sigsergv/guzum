/*
 * settings.h
 *
 * Created on: Apr 19, 2012
 * Author: Sergey Stolyarov
 */
#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <QString>
#include <QChar>
#include <QList>

class QSettings;
class QWidget;
class QString;

namespace Guzum
{
namespace Config
{
    void initSettings();
    QSettings * settings();
    QString version();
    QString uiLang();
    QString uiLangsPath();
    QString profilePath();
    QString filenameInProfile(const QString & filename);
    QString extUrlNamespace();
    QString controlSocketPath();
}
}

#endif /* SETTINGS_H_ */

