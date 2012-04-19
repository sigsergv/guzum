/*
 * settings.cpp
 *
 * Created on: Dec 27, 2011
 * Author: Sergei Stolyarov
 */

#include <QtCore>
#include <QtDebug>
#include <QWebSecurityOrigin>

#include "settings.h"

static QSettings * _settings = 0;
static QString uiLangsPath;

namespace Guzum
{
namespace Config
{
    
    QSettings * settings()
    {
        if (0 == ::_settings) {
            // init settings on first call
            QCoreApplication::setApplicationName("Guzum");
            QCoreApplication::setApplicationVersion(GUZUM_VERSION);
            QCoreApplication::setOrganizationName("regolit.com");
            QCoreApplication::setOrganizationDomain("guzum.regolit.com");

            ::_settings = new QSettings(profilePath()+"/guzum.ini", QSettings::IniFormat);

        }
        
        return ::_settings; 
    }

    QString uiLang()
    {
        return "ru";
    }

    QString uiLangsPath()
    {
        // first check for local paths
        QString localPath = QCoreApplication::applicationDirPath() + QDir::separator() + "translations";
        QDir d(localPath);
        QStringList files = d.entryList(QDir::Files);
        foreach (QString f, files) {
            if (f.endsWith(".qm")) {
                ::uiLangsPath = localPath;
                break;
            }
        }
        
        // find directory with translations
        if (::uiLangsPath.isEmpty()) {
#ifdef Q_OS_UNIX
            // check standard dirs
            QStringList checkPaths;
            checkPaths << "/usr/share/guzum/translations/";

            foreach (QString path, checkPaths) {
                QDir d(path);
                bool found = false;
                if (d.exists()) {
                    // check for *.qm files there
                    QStringList files = d.entryList(QDir::Files);
                    foreach (QString f, files) {
                        if (f.endsWith(".qm")) {
                            ::uiLangsPath = path;
                            found = true;
                            break;
                        }
                    }
                    if (found) {
                        break;
                    }
                }
            }
#endif
        }
        return ::uiLangsPath;
    }

    QString profilePath()
    {
        QDir dir;

        QString path = QDir::homePath() + "/.guzum";
        if (!dir.exists(path) && !dir.mkpath(path)) {
            // TODO: do something if it's not possible to create new directory
            return QString();
        }

        return path;
    }



    QString _cachePath(const QString & dirname)
    {
        QDir profileDir(profilePath());

        QString path = filenameInProfile(dirname);
        QFileInfo fi(path);

        if (fi.isFile()) {
            // try to remove, we don't need a filename with the same name here
            if (!profileDir.remove(dirname)) {
                return QString();
            }
        }

        if (!fi.exists()) {
            if (!profileDir.mkdir(dirname)) {
                return QString();
            }
        }

        return path;
    }


    QString filenameInProfile(const QString & filename)
    {
        QString profile = profilePath();
        QString fullFilename = profile + "/" + filename;
        QDir dir(fullFilename);

        return dir.canonicalPath();
    }

    QString extUrlNamespace()
    {
        return "guzum.ns.regolit.com";
    }

}
}

