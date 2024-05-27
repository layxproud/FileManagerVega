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
    void setPortraits(const QMap<QString, long> &portraits);

private:
    QListWidget *listWidget;
    QPushButton *addButton;
    QPushButton *removeButton;
    QMap<QString, long> portraits;

signals:
};

#endif // PORTRAITLISTWIDGET_H
