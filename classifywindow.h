#ifndef CLASSIFYWINDOW_H
#define CLASSIFYWINDOW_H

#include "portraitlistwidget.h"
#include <QCloseEvent>
#include <QDialog>
#include <QMap>

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

private:
    Ui::ClassifyWindow *ui;
    QMap<QString, long> portraits;
    QMap<QString, long> classes;
    QList<long> portraitIDs;
    QList<long> classesIDs;
    QString dbName;
    PortraitListWidget *portraitsWidget;
    PortraitListWidget *classesWidget;

    void populatePortraitsList();
    void populateClassesList();

signals:
    void getPortraitsSignal();
    void getClassesSignal();
    void classifyPortraits(const QList<long> &portraitIDs, const QList<long> &classesIDs);

private slots:
    void addPortraits();
    void onApplyButtonClicked();
};

#endif // CLASSIFYWINDOW_H
