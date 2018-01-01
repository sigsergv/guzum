/*
 * secureplaintexteditor.cpp
 *
 * Created on: Jun 09, 2012
 * Author: Sergey Stolyarov
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

void SecurePlainTextEditor::mouseReleaseEvent(QMouseEvent *)
{
    // method is required to disable text copying to X mouse selection
}

void SecurePlainTextEditor::mouseDoubleClickEvent(QMouseEvent *)
{
    // select word
    auto text = this->toPlainText();
    auto cursor = textCursor();
    auto pos = cursor.position();
    auto start = pos;
    
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

