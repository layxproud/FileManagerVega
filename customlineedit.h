#ifndef CUSTOMLINEEDIT_H
#define CUSTOMLINEEDIT_H

#include <QLineEdit>

class CustomLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit CustomLineEdit(QWidget *parent = nullptr);

protected:
    void focusOutEvent(QFocusEvent *event) override;

signals:
    void focusOut();
};

#endif // CUSTOMLINEEDIT_H
