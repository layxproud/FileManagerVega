#ifndef CLUSTERIZEWINDOW_H
#define CLUSTERIZEWINDOW_H

#include "portraitlistwidget.h"
#include <QCloseEvent>
#include <QDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMovie>
#include <QStandardItemModel>
#include <QTreeView>

namespace Ui {
class ClusterizeWindow;
}

class ClusterizeWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ClusterizeWindow(QWidget *parent = nullptr);
    ~ClusterizeWindow();

    void setPortraits(const QMap<long, QString> &p);
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
    QTreeView *resultView;

    void populateModel(QStandardItemModel *model, const QJsonObject &jsonObject);
    void writeItem(QTextStream &out, QStandardItem *item, int level);

signals:
    void addPortraitsSignal();
    void clusterizePortraits(const QList<long> &portraitIDs, int clustersNum);

private slots:
    void onApplyButtonClicked();
    void exportDataToFile();
};

#endif // CLUSTERIZEWINDOW_H
