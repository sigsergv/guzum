/*
 * secureplaintexteditor.cpp
 *
 * Created on: Jun 09, 2012
 * Author: Sergei Stolyarov
 */

#include <QtGui>
#include <QtDebug>

#include "secureplaintexteditor.h"

SecurePlainTextEditor::SecurePlainTextEditor(QWidget * parent)
    : QPlainTextEdit(parent)
{

}

SecurePlainTextEditor::~SecurePlainTextEditor()
{

}

void SecurePlainTextEditor::mouseReleaseEvent(QMouseEvent * e)
{
    // method is required to disable text copying to X mouse selection
}

void SecurePlainTextEditor::mouseDoubleClickEvent(QMouseEvent * e)
{
    // select word
    QString text = this->toPlainText();
    int textLength = text.length();
    QTextCursor cursor = textCursor();

    int pos = cursor.position();

    /*
    int end = pos;
    while (end < textLength) {
        end++;
        if (text[end].isSpace()) {
            break;
        }
    }
    */

    int start = pos;
    while (start > 0) {
        start--;
        if (text[start].isSpace()) {
            start++;
            break;
        }
    }

    cursor.setPosition(start);
    setTextCursor(cursor);
    moveCursor(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
}

