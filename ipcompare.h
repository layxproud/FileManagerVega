#ifndef IPCOMPARE_H
#define IPCOMPARE_H

#include "trmlshell.h"
#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsView>

namespace Ui {
class IPCompare;
}

class IPCompare : public QWidget
{
    Q_OBJECT

public:
    explicit IPCompare(IPCompareResults &res, QWidget *parent = nullptr);
    ~IPCompare();

private:
    Ui::IPCompare *ui;
    IPCompareResults params;
    QGraphicsScene *scene;
    void drawCirclesAndLine();
};

#endif // IPCOMPARE_H
