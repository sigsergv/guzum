/*
 * traymanager.h
 *
 * Created on: Apr 29, 2012
 * Author: Sergei Stolyarov
 */

#ifndef _TRAYMANAGER_H_
#define _TRAYMANAGER_H_

#include <QObject>

struct QAction;

class TrayManager : public QObject
{
    Q_OBJECT
public: 
    TrayManager(QObject * parent = 0);
    ~TrayManager();

protected slots:
    void quit();
    void appendFile(const QString & filename, const QString & gnupgHome);
    void openFilename();
    void setPreferences();
    void showAboutDialog();
    void menuHovered(QAction * action);

protected:
    void dumpFilenames();
    void rebuildFilenamesMenu();

private:
    struct Private;
    Private * p;
};

#endif // _TRAYMANAGER_H_
