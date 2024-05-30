#ifndef CLASSIFYWINDOW_H
#define CLASSIFYWINDOW_H

#include "portraitlistwidget.h"
#include <QCloseEvent>
#include <QDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMap>
#include <QMovie>
#include <QStandardItemModel>
#include <QTreeView>

namespace Ui {
class ClassifyWindow;
}

class ClassifyWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ClassifyWindow(QWidget *parent = nullptr);
    ~ClassifyWindow();

    void setPortraits(const QMap<QString, long> &p);
    void setClasses(const QMap<QString, long> &c);
    void setDbName(const QString &name);

protected:
    void closeEvent(QCloseEvent *event) override;

public slots:
    void onClassificationComplete(bool success, const QString &res);

private:
    Ui::ClassifyWindow *ui;
    QMap<QString, long> portraits;
    QMap<QString, long> classes;
    QString dbName;
    PortraitListWidget *portraitsWidget;
    PortraitListWidget *classesWidget;
    QMovie *movie;

    void populateModel(QStandardItemModel *model, const QJsonObject &jsonObject);

signals:
    void getPortraitsSignal();
    void getClassesSignal();
    void classifyPortraits(const QList<long> &portraitIDs, const QMap<QString, long> &classes);

private slots:
    void addPortraits();
    void onApplyButtonClicked();
};

#endif // CLASSIFYWINDOW_H
