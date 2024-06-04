#ifndef MATCHWINDOW_H
#define MATCHWINDOW_H

#include "portraitlistwidget.h"
#include "trmlshell.h"
#include <QCloseEvent>
#include <QDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMovie>
#include <QStandardItemModel>
#include <QTreeView>

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
    void setPortraits(const QMap<long, QString> &p);

public slots:
    void onFindMatchingComplete(bool success, const QString &res);

protected:
    virtual void closeEvent(QCloseEvent *event) override;

private:
    Ui::MatchWindow *ui;
    QMovie *movie;
    QTreeView *resultView;
    PortraitListWidget *portraitsWidget;
    QStandardItemModel *model;
    QString dbName;

    void populateModel(const QJsonArray &jsonArray);

signals:
    void addPortraitsSignal();
    void findMatchingPortraits(const FindMatchParams &params);

private slots:
    void onRadioButton();
    void onApplyButton();
};

#endif // MATCHWINDOW_H
