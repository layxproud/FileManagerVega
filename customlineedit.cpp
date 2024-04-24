#include "CustomLineEdit.h"

CustomLineEdit::CustomLineEdit(QWidget *parent)
    : QLineEdit(parent)
{
}

void CustomLineEdit::focusOutEvent(QFocusEvent *event)
{
    QLineEdit::focusOutEvent(event);
    emit focusOut();
}
