#include "sortableheaderview.h"
#include <QMouseEvent>

SortableHeaderView::SortableHeaderView(Qt::Orientation orientation, QWidget *parent)
    : QHeaderView(orientation, parent), sortedSection(-1), sortedOrder(Qt::AscendingOrder)
{
    setSortIndicatorShown(true);
}

void SortableHeaderView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        int logicalIndex = logicalIndexAt(event->pos());

        if (logicalIndex >= 0 && logicalIndex < count()) {
            sortedSection = logicalIndex;
            sortedOrder = (sortedOrder == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;
            sortOrderMap.insert(logicalIndex, sortedOrder);
            setSortIndicator(logicalIndex, sortedOrder);
            emit sortIndicatorChanged(logicalIndex, sortedOrder);
        }
    }
    QHeaderView::mousePressEvent(event);
}
