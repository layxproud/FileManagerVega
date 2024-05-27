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
    connect(
        ui->addPortraitButton,
        &QPushButton::clicked,
        this,
        &ClassifyWindow::onAddPortraitButtonClicked);
    connect(
        ui->removePortraitButton,
        &QPushButton::clicked,
        this,
        &ClassifyWindow::onRemovePortraitButtonClicked);

    connect(ui->cancelButton, &QPushButton::clicked, this, &ClassifyWindow::close);
    connect(ui->applyButton, &QPushButton::clicked, this, &ClassifyWindow::onApplyButtonClicked);
}

ClassifyWindow::~ClassifyWindow()
{
    delete ui;
}

void ClassifyWindow::populatePortraitsList()
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

void ClassifyWindow::populateClassesList()
{
    ui->classesList->clear();
    for (auto it = classes.begin(); it != classes.end(); ++it) {
        const QString &name = it.key();
        long id = it.value();

        QListWidgetItem *item = new QListWidgetItem(name);

        QVariant idData(id);
        item->setData(Qt::UserRole, idData);

        ui->classesList->addItem(item);
    }
}

void ClassifyWindow::setPortraits(const QMap<QString, long> &p)
{
    for (auto it = p.begin(); it != p.end(); ++it) {
        if (!portraits.contains(it.key())) {
            portraits.insert(it.key(), it.value());
        }
    }

    populatePortraitsList();
}

void ClassifyWindow::setClasses(const QMap<QString, long> &c)
{
    for (auto it = c.begin(); it != c.end(); ++it) {
        if (!classes.contains(it.key())) {
            classes.insert(it.key(), it.value());
        }
    }

    populateClassesList();
}

void ClassifyWindow::setDbName(const QString &name)
{
    dbName = name;
    ui->dbNameInput->setText(dbName);
}

void ClassifyWindow::closeEvent(QCloseEvent *event)
{
    deleteLater();
    event->accept();
}

void ClassifyWindow::onAddClassButtonClicked()
{
    emit getClassesSignal();
}

void ClassifyWindow::onRemoveClassButtonClicked()
{
    QListWidgetItem *selectedItem = ui->portraitsList->currentItem();
    if (selectedItem) {
        QString className = selectedItem->text();

        auto it = classes.find(className);
        if (it != classes.end()) {
            classes.erase(it);
        }

        int row = ui->classesList->row(selectedItem);
        delete ui->classesList->takeItem(row);
    }
}

void ClassifyWindow::onAddPortraitButtonClicked()
{
    emit getPortraitsSignal();
}

void ClassifyWindow::onRemovePortraitButtonClicked()
{
    QListWidgetItem *selectedItem = ui->portraitsList->currentItem();
    if (selectedItem) {
        QString portraitName = selectedItem->text();

        auto it = portraits.find(portraitName);
        if (it != portraits.end()) {
            portraits.erase(it);
        }

        int row = ui->portraitsList->row(selectedItem);
        delete ui->portraitsList->takeItem(row);
    }
}

void ClassifyWindow::onApplyButtonClicked()
{
    if (ui->classesList->count() == 0 || ui->portraitsList->count() == 0) {
        qWarning() << "Не выбрано ни одного класса или портрета";
        return;
    }

    portraitIDs.clear();
    classesIDs.clear();

    for (int i = 0; i < ui->portraitsList->count(); ++i) {
        QListWidgetItem *item = ui->portraitsList->item(i);
        QVariant idData = item->data(Qt::UserRole);
        portraitIDs.append(idData.toLongLong());
    }

    for (int i = 0; i < ui->classesList->count(); ++i) {
        QListWidgetItem *item = ui->classesList->item(i);
        QVariant idData = item->data(Qt::UserRole);
        classesIDs.append(idData.toLongLong());
    }

    emit classifyPortraits(portraitIDs, classesIDs);
}
