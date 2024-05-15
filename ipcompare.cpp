#include "ipcompare.h"
#include "ui_ipcompare.h"
#include <QDebug>

IPCompare::IPCompare(IPCompareResults &res, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::IPCompare),
    params(res)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    drawCirclesAndLine();
    updateLabels();
}

IPCompare::~IPCompare()
{
    qDebug() << "Deleting";
    delete ui;
}

void IPCompare::drawCirclesAndLine()
{
    QGraphicsEllipseItem* circle1 = new QGraphicsEllipseItem(-params.r1, -params.r1, 2 * params.r1, 2 * params.r1);
    QGraphicsEllipseItem* circle2 = new QGraphicsEllipseItem(-params.r2, -params.r2, 2 * params.r2, 2 * params.r2);

    // Set positions of circles
    circle1->setPos(0, 0);
    circle2->setPos(params.d, 0); // Assuming the distance is from the center of circle1 to the center of circle2

    // Create line representing the distance between centers
    QGraphicsLineItem* line = new QGraphicsLineItem(0, 0, params.d, 0);

    // Add items to the scene
    scene->addItem(circle1);
    scene->addItem(circle2);
    scene->addItem(line);
}

void IPCompare::updateLabels()
{
    ui->c1Value->setText(QString::number(params.comm1));
    ui->c2Value->setText(QString::number(params.comm2));
    ui->d1Value->setText(QString::number(params.diff1));
    ui->d2Value->setText(QString::number(params.diff2));
}
