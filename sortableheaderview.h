#ifndef SORTABLEHEADERVIEW_H
#define SORTABLEHEADERVIEW_H

#include <QHeaderView>
#include <QMouseEvent>

class SortableHeaderView : public QHeaderView
{
    Q_OBJECT
public:
    explicit SortableHeaderView(Qt::Orientation orientation, QWidget* parent = nullptr);

signals:
    void sortIndicatorChanged(int logicalIndex, Qt::SortOrder order);

protected:
    void mousePressEvent(QMouseEvent* event) override;

private:
    QMap<int, Qt::SortOrder> sortOrderMap;
    int sortedSection;
    Qt::SortOrder sortedOrder;
};

#endif // SORTABLEHEADERVIEW_H
