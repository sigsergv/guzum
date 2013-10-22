/*
 * traymanager.h
 *
 * Created on: Apr 29, 2012
 * Author: Sergey Stolyarov
 */

#ifndef _TRAYMANAGER_H_
#define _TRAYMANAGER_H_

#include <QObject>

class QAction;

class TrayManager : public QObject
{
    Q_OBJECT
public: 
    static TrayManager * instance();
    ~TrayManager();

public slots:
    void appendFile(const QString & filename, const QString & gnupgHome);

protected slots:
    void quit();
    void openFilename();
    void setPreferences();
    void showAboutDialog();
    void manageHistory();
    void menuHovered(QAction * action);

protected:
    void dumpFilenames();
    void rebuildFilenamesMenu();
    void reloadFilenames();

private:
    static TrayManager * inst;
    TrayManager(QObject * parent = 0);
    struct Private;
    Private * p;
};

#endif // _TRAYMANAGER_H_
