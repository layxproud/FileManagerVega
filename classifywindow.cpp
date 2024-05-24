#include "classifywindow.h"
#include "ui_classifywindow.h"
#include <QDebug>

ClassifyWindow::ClassifyWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ClassifyWindow)
{
    ui->setupUi(this);

    connect(ui->addClassButton, &QPushButton::clicked, this, &ClassifyWindow::onAddClassButtonClicked);
    connect(
        ui->removeClassButton,
        &QPushButton::clicked,
        this,
        &ClassifyWindow::onRemoveClassButtonClicked);

    connect(ui->cancelButton, &QPushButton::clicked, this, &ClassifyWindow::close);
    connect(ui->applyButton, &QPushButton::clicked, this, &ClassifyWindow::onApplyButtonClicked);
}

ClassifyWindow::~ClassifyWindow()
{
    delete ui;
}

void ClassifyWindow::populateListWidget()
{
    ui->portraitsList->clear();
    for (auto it = portraits.begin(); it != portraits.end(); ++it) {
        const QString &name = it.key();
        long id = it.value();

        QListWidgetItem *item = new QListWidgetItem(name);

        QVariant idData(id);
        item->setData(Qt::UserRole, idData);

        ui->portraitsList->addItem(item);
    }
}

void ClassifyWindow::setPortraits(const QMap<QString, long> &p)
{
    portraits.clear();
    portraits = p;

    populateListWidget();
}

void ClassifyWindow::setDbName(const QString &name)
{
    dbName = name;
    ui->dbNameInput->setText(dbName);
}

void ClassifyWindow::onAddClassButtonClicked()
{
    CustomItemWidget *itemWidget = new CustomItemWidget();
    CustomListItem *item = new CustomListItem(itemWidget);
    ui->classesList->addItem(item);
    ui->classesList->setItemWidget(item, itemWidget);
}

void ClassifyWindow::onRemoveClassButtonClicked()
{
    QListWidgetItem *selectedItem = ui->classesList->currentItem();
    if (selectedItem) {
        int row = ui->classesList->row(selectedItem);
        delete ui->classesList->takeItem(row);
    }
}

void ClassifyWindow::onApplyButtonClicked()
{
    ids.clear();

    for (int i = 0; i < ui->portraitsList->count(); ++i) {
        QListWidgetItem *item = ui->portraitsList->item(i);
        QVariant idData = item->data(Qt::UserRole);
        ids.append(idData.toLongLong());
    }
    qDebug() << ids;
}
