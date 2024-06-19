#ifndef PORTRAITLISTWIDGET_H
#define PORTRAITLISTWIDGET_H

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMap>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

class PortraitListWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PortraitListWidget(QWidget *parent = nullptr);

    void setPortraits(const QMap<long, QString> &items);
    QMap<long, QString> getPortraits() const;
    void clear();

private:
    QListWidget *listWidget;
    QPushButton *addButton;
    QPushButton *removeButton;
    QLabel *portraitsNumValue;
    QMap<long, QString> portraits;

    void populateList();

signals:
    void addPortraitsSignal();

private slots:
    void onRemoveButtonClicked();
};

#endif // PORTRAITLISTWIDGET_H
