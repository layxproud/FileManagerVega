#ifndef VIEWIP_H
#define VIEWIP_H

#include <QWidget>
#include <QString>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsRectItem>

#include <vector>

#define HEADER "ID\nСлово\nВес\nВхождений"

using namespace std;


namespace Ui {
class Widget;
}

struct IPTerm;
struct IPShingle;

/*
struct IPTerm
{
    ulong			id;				//
    QString 		term;			//
    double			weight;			//

    IPTerm(ulong i, QString t, double w) {id = i, term = t, weight = w; }
};

struct IPShingle
{
    ulong			id;				//
    QString 		term;			//
    double			weight;			//

    IPShingle(ulong i, QString t, double w) {id = i, term = t, weight = w; }
};
*/

class CustomGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit CustomGraphicsView(QWidget *parent = nullptr) : QGraphicsView(parent) {}

signals:
    void resized();

protected:
    void resizeEvent(QResizeEvent *event) override {
        QGraphicsView::resizeEvent(event);
        emit resized();
    }
};

class ViewIP : public QWidget
{
    Q_OBJECT

public:
    explicit ViewIP(QWidget *parent = 0);
    ~ViewIP();

    void setData(QString id, QString user, QString date, QString comment, vector<IPTerm*> terms, vector<IPShingle*> shingles);


private:
    Ui::Widget *ui;
    QGraphicsScene* scene;
    double totalWeight = 0.0;
    std::vector<IPTerm*> top10Terms;
    std::vector<double> percentages;
    void fillRectangle();
    void updateSceneSize();

public slots:
//    void genData();
    void clear();
};

#endif // VIEWIP_H
