#ifndef CLUSTERIZEWINDOW_H
#define CLUSTERIZEWINDOW_H

#include "portraitlistwidget.h"
#include <QCloseEvent>
#include <QDialog>

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

private:
    Ui::ClusterizeWindow *ui;
    PortraitListWidget *portraitsWidget;
    QString dbName;

signals:
    void addPortraitsSignal();
    void clusterizePortraits(const QList<long> &portraitIDs, int clustersNum);

private slots:
    void onApplyButtonClicked();
};

#endif // CLUSTERIZEWINDOW_H
