#ifndef CLASSIFYWINDOW_H
#define CLASSIFYWINDOW_H

#include <QDialog>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QListWidgetItem>
#include <QWidget>

class CustomItemWidget : public QWidget
{
    Q_OBJECT
public:
    CustomItemWidget(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        lineEdit1 = new QLineEdit(this);
        lineEdit2 = new QLineEdit(this);
        QFont font("Segoe UI", 12);
        lineEdit1->setFont(font);
        lineEdit2->setFont(font);
        QHBoxLayout *layout = new QHBoxLayout;
        layout->addWidget(lineEdit1);
        layout->addWidget(lineEdit2);
        setLayout(layout);
    }

    QLineEdit *lineEdit1;
    QLineEdit *lineEdit2;
};

class CustomListItem : public QListWidgetItem
{
public:
    CustomListItem(CustomItemWidget *widget)
        : QListWidgetItem()
        , customWidget(widget)
    {
        setSizeHint(customWidget->sizeHint());
    }

    CustomItemWidget *customWidget;
};

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
    void setDbName(const QString &name);

private:
    Ui::ClassifyWindow *ui;
    QMap<QString, long> portraits;
    QMap<QString, long> classes;
    QList<long> ids;
    QString dbName;

    void populateListWidget();

signals:
    void classifyPortraits(const QList<long> &ids, const QMap<QString, long> &classes);

private slots:
    void onAddClassButtonClicked();
    void onRemoveClassButtonClicked();
    void onApplyButtonClicked();
};

#endif // CLASSIFYWINDOW_H
