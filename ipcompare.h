#ifndef IPCOMPARE_H
#define IPCOMPARE_H

#include "trmlshell.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QWidget>

namespace Ui {
class IPCompare;
}

class IPCompare : public QWidget
{
    Q_OBJECT

public:
    explicit IPCompare(const IPCompareResults &res, QWidget *parent = nullptr);
    ~IPCompare();

private:
    Ui::IPCompare *ui;
    IPCompareResults params;
    QGraphicsScene *scene;
    void drawCirclesAndLine();
    void updateLabels();
};

#endif // IPCOMPARE_H
