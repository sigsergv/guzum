/*
 * settings.cpp
 *
 * Created on: Dec 27, 2011
 * Author: Sergey Stolyarov
 */

#include <QtCore>
#include <QtDebug>

#include "settings.h"

static QSettings * _settings = 0;
static QString uiLangsPath;
static QApplication * _app = 0;
static QTranslator * _translator = 0;

namespace Guzum
{
namespace Config
{

static const QStringList ALLOWED_LANGUAGES = {"en", "ru"};

// explicitly initialize settings
void initSettings(QApplication * app)
{
    // init settings on first call
    QCoreApplication::setApplicationName("Guzum");
    QCoreApplication::setApplicationVersion(GUZUM_VERSION);
    QCoreApplication::setOrganizationName("regolit.com");
    QCoreApplication::setOrganizationDomain("guzum.regolit.com");

    ::_settings = new QSettings(profilePath() + "/guzum.ini", QSettings::IniFormat);
    ::_app = app;

    ::_translator = new QTranslator();
    ::_translator->load(":/translators/"+uiLang()+".qm");
    QCoreApplication::installTranslator(::_translator);
}

QSettings * settings()
{
    return ::_settings; 
}

QTranslator * translator()
{
    return ::_translator;
}

QString uiLang()
{
    auto s = settings();

    s->beginGroup("ui");
    auto lang = s->value("language", "en").toString();
    if (!ALLOWED_LANGUAGES.contains(lang)) {
        lang = "en";
    }
    s->endGroup();

    return lang;
}

void setUiLang(const QString & newLang)
{
    auto lang = newLang;
    if (!ALLOWED_LANGUAGES.contains(lang)) {
        lang = "en";
    }

    auto s = settings();

    s->beginGroup("ui");
    s->setValue("language", lang);
    s->endGroup();

    ::_translator->load(":/translators/"+ lang +".qm");
}

QString uiLangsPath()
{
    // first check for local paths
    QString localPath = QCoreApplication::applicationDirPath() + QDir::separator() + "translations";
    QDir d(localPath);
    auto files = d.entryList(QDir::Files);
    foreach (QString f, files) {
        if (f.endsWith(".qm")) {
            ::uiLangsPath = localPath;
            break;
        }
    }
    
    // find directory with translations
    if (::uiLangsPath.isEmpty()) {
#ifdef Q_OS_LINUX
        // check standard dirs
        QStringList checkPaths;
        checkPaths << "/usr/share/guzum/translations/";

        foreach (QString path, checkPaths) {
            QDir d(path);
            auto found = false;
            if (d.exists()) {
                // check for *.qm files there
                auto files = d.entryList(QDir::Files);
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
    QString configHome = qgetenv("XDG_CONFIG_HOME");

    if (configHome.size() == 0) {
        configHome = QDir::homePath() + "/.config";
    }
    QDir configHomeDir(configHome);

    if (!configHomeDir.exists("guzum")) {
        // TODO: check return value
        configHomeDir.mkpath("guzum");
    }

    return configHome + "/guzum";
}


QString _cachePath(const QString & dirname)
{
    QDir profileDir(profilePath());

    auto path = filenameInProfile(dirname);
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
    auto profile = profilePath();
    auto fullFilename = profile + "/" + filename;
    QDir dir(fullFilename);

    return dir.canonicalPath();
}

QString extUrlNamespace()
{
    return "guzum.ns.regolit.com";
}

QString controlSocketPath()
{
    return profilePath() + "/control_socket";
}

QApplication * app()
{
    return ::_app;
}

}
}

