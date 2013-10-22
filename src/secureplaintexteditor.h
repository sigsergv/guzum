/*
 * secureplaintexteditor.h
 *
 * Created on: Jun 09, 2012
 * Author: Sergey Stolyarov
 */

#ifndef _SECUREPLAINTEXTEDITOR_H_
#define _SECUREPLAINTEXTEDITOR_H_

#include <QPlainTextEdit>

class QMouseEvent;

class SecurePlainTextEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    SecurePlainTextEditor(QWidget * parent = 0);
    ~SecurePlainTextEditor();

protected:
    virtual void mouseReleaseEvent(QMouseEvent * e);
    virtual void mouseDoubleClickEvent(QMouseEvent * e);
};

#endif // _SECUREPLAINTEXTEDITOR_H_
