#ifndef MATCHWINDOW_H
#define MATCHWINDOW_H

#include "portraitlistwidget.h"
#include "trmlshell.h"
#include <QCloseEvent>
#include <QDialog>

namespace Ui {
class MatchWindow;
}

class MatchWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MatchWindow(QWidget *parent = nullptr);
    ~MatchWindow();

    void setDbName(const QString &name) { dbName = name; }
    void setPortraits(const QMap<QString, long> &p);

protected:
    virtual void closeEvent(QCloseEvent *event) override;

private:
    Ui::MatchWindow *ui;
    PortraitListWidget *portraitsWidget;
    QString dbName;

signals:
    void addPortraitsSignal();
    void findMatchingPortraits(const FindMatchParams &params);

private slots:
    void onRadioButton();
    void onApplyButton();
};

#endif // MATCHWINDOW_H
