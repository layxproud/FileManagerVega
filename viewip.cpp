#include "viewip.h"
#include "ui_viewip.h"
#include "trmlshell.h"
#include <QDebug>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    ui->tb_terms->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tb_terms->horizontalHeader()->setStretchLastSection(true);
    ui->tb_terms->verticalHeader()->hide();

    ui->tb_shingles->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tb_shingles->horizontalHeader()->setStretchLastSection(true);
    ui->tb_shingles->verticalHeader()->hide();

    QStringList horHeader = QString(HEADER).split("\n");

    ui->tb_terms->setColumnCount(4);
    ui->tb_terms->setHorizontalHeaderLabels(horHeader);

    ui->tb_shingles->setColumnCount(4);
    ui->tb_shingles->setHorizontalHeaderLabels(horHeader);

    int iColWidths[3] = {100, 150, 100};
    for (int i = 0; i < 3; ++i)
    {
        ui->tb_terms->setColumnWidth(i, iColWidths[i]);
        ui->tb_shingles->setColumnWidth(i, iColWidths[i]);
    }

    connect(ui->pb_set, SIGNAL(clicked()), this, SLOT(genData()));
    connect(ui->pb_clear, SIGNAL(clicked()), this, SLOT(clear()));

    ui->tb_terms->setSortingEnabled(true);
    ui->tb_shingles->setSortingEnabled(true);

    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    scene->setSceneRect(ui->graphicsView->viewport()->rect());
}

Widget::~Widget()
{
    delete ui;
}
/*
std::string random_string( size_t length )
{
    auto randchar = []() -> char
    {
        const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[ rand() % max_index ];
    };
    std::string str(length,0);
    std::generate_n( str.begin(), length, randchar );
    return str;
}

void Widget::genData()
{
    vector<IPTerm*> terms;
    for(int i = 0; i < rand() % 100; i++)
    {
        IPTerm *p = new IPTerm(static_cast<ulong>(i+1), QString(random_string(20).c_str()), (double)rand() / RAND_MAX);
        terms.push_back(p);
    }
    vector<IPShingle*> shingles;
    for(int i = 0; i < rand() % 100; i++)
    {
        IPShingle *p = new IPShingle(static_cast<ulong>(i+1), QString(random_string(15).c_str()), (double)rand() / RAND_MAX);
        shingles.push_back(p);
    }
    setData("256", "ПОЛЬЗОВАТЕЛЬ", "11.02", "Комментарий", terms, shingles);
}
*/
void Widget::setData(QString id, QString user, QString date, QString comment, vector<IPTerm*> terms, vector<IPShingle*> shingles)
{
    totalWeight = 0.0;
    top10Terms.clear();
    percentages.clear();
    ui->edit_id->setText(id);
    ui->edit_user->setText(user);
    ui->edit_date->setText(date);
    ui->edit_com->setText(comment);


    for(uint i = 0; i < terms.size(); i++)
    {
         if(ui->tb_terms->rowCount() <= i) ui->tb_terms->insertRow(i);
         ui->tb_terms->setItem(i,0, new QTableWidgetItem(QString("%1").arg(terms[i]->id)));
         ui->tb_terms->setItem(i,1, new QTableWidgetItem(terms[i]->term));

         QVariant weightData = QVariant::fromValue(static_cast<double>(terms[i]->weight));
         ui->tb_terms->setItem(i, 2, new QTableWidgetItem);
         ui->tb_terms->item(i, 2)->setData(Qt::EditRole, weightData);

         QVariant timesData = QVariant::fromValue(static_cast<int>(terms[i]->times));
         ui->tb_terms->setItem(i, 3, new QTableWidgetItem);
         ui->tb_terms->item(i, 3)->setData(Qt::EditRole, timesData);

         totalWeight += terms[i]->weight;
    }

    for(uint i = 0; i < shingles.size(); i++)
    {
        if(ui->tb_shingles->rowCount() <= i) ui->tb_shingles->insertRow(i);
        ui->tb_shingles->setItem(i,0, new QTableWidgetItem(QString("%1").arg(shingles[i]->id)));
        ui->tb_shingles->setItem(i,1, new QTableWidgetItem(shingles[i]->term));
        ui->tb_shingles->setItem(i,2, new QTableWidgetItem(QString("%1").arg(shingles[i]->weight)));
        ui->tb_shingles->setItem(i,3, new QTableWidgetItem(QString("%1").arg(shingles[i]->times)));
    }

    std::sort(terms.begin(), terms.end(), [](const IPTerm* a, const IPTerm* b) {
        return a->weight > b->weight;
    });
    top10Terms = {terms.begin(), terms.begin() + std::min(10, static_cast<int>(terms.size()))};
    int curr = 0;
    for (const auto& term : top10Terms) {
        double percentage = (term->weight / totalWeight) * 100.0;
        percentages.push_back(percentage);

        qDebug() << "Term " << curr << " weight is " << percentage;
        curr++;
    }

    fillRectangle();
}

void Widget::fillRectangle()
{
    // Get the scene's width and height
    auto sceneWidth = scene->width();
    auto sceneHeight = scene->height();

    // Create a rectangle that fills the entire scene
    QGraphicsRectItem* rect = scene->addRect(0, 0, sceneWidth, sceneHeight);

    // Fill the rectangle with colors based on percentages
    double currentX = 0;
    for (size_t i = 0; i < percentages.size(); ++i) {
        double segmentWidth = percentages[i] * sceneWidth / 100.0;
        QColor color;
        switch (i) {
        case 0: color = Qt::red; break;
        case 1: color = Qt::blue; break;
        case 2: color = Qt::green; break;
        case 3: color = Qt::red; break;
        case 4: color = Qt::blue; break;
        case 5: color = Qt::green; break;
        case 6: color = Qt::red; break;
        case 7: color = Qt::blue; break;
        case 8: color = Qt::green; break;
        case 9: color = Qt::red; break;
            // Add more colors as needed
        }
        QGraphicsRectItem* filledRect = scene->addRect(currentX, 0, segmentWidth, sceneHeight, QPen(), QBrush(color));
        currentX += segmentWidth;
    }
}

void Widget::clear()
{
    ui->edit_id->clear();
    ui->edit_user->clear();
    ui->edit_date->clear();
    ui->edit_com->clear();
    ui->tb_terms->clearContents();
    ui->tb_shingles->clearContents();
}
