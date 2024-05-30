#ifndef PORTRAITLISTWIDGET_H
#define PORTRAITLISTWIDGET_H

#include <QHBoxLayout>
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

    void setItems(const QMap<long, QString> &items);
    QMap<long, QString> getItems() const;
    void clear();

private:
    QListWidget *listWidget;
    QPushButton *addButton;
    QPushButton *removeButton;
    QMap<long, QString> portraits;

    void onRemoveButtonClicked();
    void populateList();

signals:
    void addPortraitsSignal();
};

#endif // PORTRAITLISTWIDGET_H
