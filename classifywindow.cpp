#include "classifywindow.h"
#include "ui_classifywindow.h"
#include <QDebug>
#include <QHeaderView>

ClassifyWindow::ClassifyWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ClassifyWindow)
    , portraitsWidget(new PortraitListWidget(this))
    , classesWidget(new PortraitListWidget(this))
    , resultView(new QTreeView())
{
    ui->setupUi(this);

    ui->portraitsLayout->addWidget(portraitsWidget);
    ui->classesLayout->addWidget(classesWidget);

    movie = new QMovie(":/gifs/resources/loading.gif");

    connect(ui->cancelButton, &QPushButton::clicked, this, &ClassifyWindow::close);
    connect(ui->applyButton, &QPushButton::clicked, this, &ClassifyWindow::onApplyButtonClicked);
    connect(
        portraitsWidget,
        &PortraitListWidget::addPortraitsSignal,
        this,
        &ClassifyWindow::addPortraits);
    connect(
        classesWidget, &PortraitListWidget::addPortraitsSignal, this, &ClassifyWindow::addPortraits);
}

ClassifyWindow::~ClassifyWindow()
{
    delete resultView;
    delete ui;
}

void ClassifyWindow::setPortraits(const QMap<long, QString> &p)
{
    portraitsWidget->setItems(p);
}

void ClassifyWindow::setClasses(const QMap<long, QString> &c)
{
    classesWidget->setItems(c);
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

void ClassifyWindow::onClassificationComplete(bool success, const QString &res)
{
    ui->loadingLabel->movie()->stop();
    ui->loadingLabel->clear();

    if (!success) {
        ui->loadingLabel->setText("ОШИБКА");
        ui->loadingLabel->setStyleSheet("QLabel { font-size: 20px; color : red; }");
    } else {
        ui->loadingLabel->setText("");

        QJsonDocument jsonResponse = QJsonDocument::fromJson(res.toUtf8());
        if (jsonResponse.isNull() || !jsonResponse.isObject()) {
            qCritical() << "Ошибка при разборе JSON ответа";
            ui->loadingLabel->setText("ОШИБКА");
            ui->loadingLabel->setStyleSheet("QLabel { font-size: 20px; color : red; }");
            return;
        }

        QJsonObject jsonObject = jsonResponse.object();

        QStandardItemModel *model = new QStandardItemModel(this);
        model->setHorizontalHeaderLabels({"Классификация"});

        populateModel(model, jsonObject);

        if (ui->resultLayout->indexOf(resultView) == -1) {
            ui->resultLayout->addWidget(resultView);
        } else {
            while (QLayoutItem *item = ui->resultLayout->takeAt(0)) {
                if (item->widget()) {
                    item->widget()->setParent(nullptr);
                }
                delete item;
            }
            ui->resultLayout->addWidget(resultView);
        }

        resultView->setModel(model);
        resultView->expandAll();

        resultView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        resultView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        resultView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
        resultView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        resultView->resizeColumnToContents(0);
    }
}

void ClassifyWindow::populateModel(QStandardItemModel *model, const QJsonObject &jsonObject)
{
    QMap<long, QString> portraitsMap = portraitsWidget->getItems();

    for (auto it = jsonObject.begin(); it != jsonObject.end(); ++it) {
        QStandardItem *parentItem = new QStandardItem(it.key());
        QJsonArray jsonArray = it.value().toArray();
        for (const QJsonValue &value : jsonArray) {
            long id = value.toInt();
            QString displayValue = portraitsMap.contains(id) ? portraitsMap.value(id)
                                                             : QString::number(id);
            QStandardItem *childItem = new QStandardItem(displayValue);
            parentItem->appendRow(childItem);
        }
        model->appendRow(parentItem);
    }
}

void ClassifyWindow::addPortraits()
{
    PortraitListWidget *senderWidget = qobject_cast<PortraitListWidget *>(sender());
    if (senderWidget == portraitsWidget) {
        emit getPortraitsSignal();
    } else if (senderWidget == classesWidget) {
        emit getClassesSignal();
    }
}

void ClassifyWindow::onApplyButtonClicked()
{
    if (portraitsWidget->getItems().isEmpty() || classesWidget->getItems().isEmpty()) {
        qWarning() << "Не выбрано ни одного класса или портрета";
        return;
    }

    QList<long> portraitIDs;

    QMap<long, QString> portraitsMap = portraitsWidget->getItems();
    for (auto it = portraitsMap.begin(); it != portraitsMap.end(); ++it) {
        portraitIDs.append(it.key());
    }

    QMap<long, QString> classesMap = classesWidget->getItems();

    emit classifyPortraits(portraitIDs, classesMap);
    ui->loadingLabel->setMovie(movie);
    ui->loadingLabel->movie()->start();
}
