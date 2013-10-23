/*
 * encryptedtextwindow.h
 *
 * Created on: Apr 19, 2012
 * Author: Sergey Stolyarov
 */

#ifndef _ENCRYPTEDTEXTWINDOW_H_
#define _ENCRYPTEDTEXTWINDOW_H_

#include <QMainWindow>

class QString;
class QEvent;

class EncryptedTextWindow : public QMainWindow
{
    Q_OBJECT
public:
    EncryptedTextWindow(const QString & filename, const QString & gnupgHome, QWidget * parent = 0);
    ~EncryptedTextWindow();

private:
    struct Private;
    Private * p;

public slots:
    void close();
    bool show();

protected slots:
    void showAboutDialog();
    void saveFile();
    void changeCurrentFont();
    void changeDefaultFont();
    void insertRandomString();
    void editorModificationChanged(bool changed);
    void closeTimerTick();

protected:
    void rememberGeometryAndState();
    void moveEvent(QMoveEvent * event);
    void resizeEvent(QResizeEvent * event);
    void closeEvent(QCloseEvent * event);
    virtual bool eventFilter(QObject *obj, QEvent *event);
};

#endif // _ENCRYPTEDTEXTWINDOW_H_
