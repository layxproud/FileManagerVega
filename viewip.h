#ifndef VIEWIP_H
#define VIEWIP_H

#include <QWidget>
#include <QString>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QCloseEvent>
#include <vector>

#define HEADER "ID\nСлово\nВес\nВхождений"

using namespace std;


namespace Ui {
class Widget;
}

struct IPTerm;
struct IPShingle;

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

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::Widget *ui;
    QGraphicsScene *scene;
    double totalTermsWeight;
    double totalShinglesWeight;
    std::vector<IPTerm*> top10Terms;
    std::vector<IPShingle*> top10Shingles;
    std::vector<double> percentages;

private slots:
    void fillRectangle();
    void updateSceneSize();
    void onTabChanged(int index);
    void clear();
};

#endif // VIEWIP_H
