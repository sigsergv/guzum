/*
 * encryptedtextwindow.h
 *
 * Created on: Apr 19, 2012
 * Author: Sergei Stolyarov
 */

#ifndef _ENCRYPTEDTEXTWINDOW_H_
#define _ENCRYPTEDTEXTWINDOW_H_

#include <QMainWindow>

struct QString;

class EncryptedTextWindow : public QMainWindow
{
    Q_OBJECT
public:
    EncryptedTextWindow(const QString & filename, QWidget * parent = 0);
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

protected:
    void rememberGeometryAndState();
    void moveEvent(QMoveEvent * event);
    void resizeEvent(QResizeEvent * event);
    void closeEvent(QCloseEvent * event);
};

#endif // _ENCRYPTEDTEXTWINDOW_H_
