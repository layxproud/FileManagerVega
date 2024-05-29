#ifndef CLUSTERIZEWINDOW_H
#define CLUSTERIZEWINDOW_H

#include "portraitlistwidget.h"
#include <QCloseEvent>
#include <QDialog>
#include <QMovie>
#include <QTextBrowser>

namespace Ui {
class ClusterizeWindow;
}

class ClusterizeWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ClusterizeWindow(QWidget *parent = nullptr);
    ~ClusterizeWindow();

    void setPortraits(const QMap<QString, long> &p);
    void setDbName(const QString &name);

protected:
    void closeEvent(QCloseEvent *event) override;

public slots:
    void onClusterizationComplete(bool success, const QString &res);

private:
    Ui::ClusterizeWindow *ui;
    PortraitListWidget *portraitsWidget;
    QString dbName;
    QMovie *movie;
    QTextBrowser *browser;

signals:
    void addPortraitsSignal();
    void clusterizePortraits(const QList<long> &portraitIDs, int clustersNum);

private slots:
    void onApplyButtonClicked();
};

#endif // CLUSTERIZEWINDOW_H
