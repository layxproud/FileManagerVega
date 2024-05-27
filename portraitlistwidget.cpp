#include "portraitlistwidget.h"

PortraitListWidget::PortraitListWidget(QWidget *parent)
    : QWidget{parent}
{
    listWidget = new QListWidget(this);
    addButton = new QPushButton("+", this);
    removeButton = new QPushButton("-", this);

    connect(addButton, &QPushButton::clicked, this, &PortraitListWidget::addPortraitsSignal);
    connect(removeButton, &QPushButton::clicked, this, &PortraitListWidget::onRemoveButtonClicked);

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget(removeButton);
    buttonsLayout->addWidget(addButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(buttonsLayout);
    mainLayout->addWidget(listWidget);

    listWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    addButton->setObjectName("addButton");
    addButton->setStyleSheet("#addButton {"
                             "background-color: rgb(0, 186, 0);"
                             "color: white;"
                             "}"
                             "#addButton:hover {"
                             "background-color: rgb(0, 140, 0);"
                             "}");

    removeButton->setObjectName("removeButton");
    removeButton->setStyleSheet("#removeButton {"
                                "background-color: rgb(255, 105, 105);"
                                "color: white;"
                                "}"
                                "#removeButton:hover {"
                                "background-color: rgb(191, 79, 79);"
                                "}");
}

void PortraitListWidget::setItems(const QMap<QString, long> &items)
{
    for (auto it = items.begin(); it != items.end(); ++it) {
        if (!portraits.contains(it.key())) {
            portraits.insert(it.key(), it.value());
        }
    }

    populateList();
}

QMap<QString, long> PortraitListWidget::getItems() const
{
    QMap<QString, long> items;
    for (int i = 0; i < listWidget->count(); ++i) {
        QListWidgetItem *item = listWidget->item(i);
        items.insert(item->text(), item->data(Qt::UserRole).toLongLong());
    }
    return items;
}

void PortraitListWidget::onRemoveButtonClicked()
{
    QList<QListWidgetItem *> selectedItems = listWidget->selectedItems();
    foreach (QListWidgetItem *item, selectedItems) {
        QString portraitName = item->text();
        auto it = portraits.find(portraitName);
        if (it != portraits.end()) {
            portraits.erase(it);
        }
        delete listWidget->takeItem(listWidget->row(item));
    }
}

void PortraitListWidget::populateList()
{
    listWidget->clear();
    for (auto it = portraits.begin(); it != portraits.end(); ++it) {
        const QString &name = it.key();
        long id = it.value();

        QListWidgetItem *item = new QListWidgetItem(name);

        QVariant idData(id);
        item->setData(Qt::UserRole, idData);

        listWidget->addItem(item);
    }
}
