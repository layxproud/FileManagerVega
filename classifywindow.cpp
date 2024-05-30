#include "classifywindow.h"
#include "ui_classifywindow.h"
#include <QDebug>

ClassifyWindow::ClassifyWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ClassifyWindow)
    , portraitsWidget(new PortraitListWidget(this))
    , classesWidget(new PortraitListWidget(this))
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
    delete ui;
}

void ClassifyWindow::setPortraits(const QMap<QString, long> &p)
{
    portraitsWidget->setItems(p);
}

void ClassifyWindow::setClasses(const QMap<QString, long> &c)
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

        QTreeView *treeView = new QTreeView(this);
        treeView->setModel(model);
        treeView->expandAll();

        ui->resultLayout->addWidget(treeView);
    }
}

void ClassifyWindow::populateModel(QStandardItemModel *model, const QJsonObject &jsonObject)
{
    for (auto it = jsonObject.begin(); it != jsonObject.end(); ++it) {
        QStandardItem *parentItem = new QStandardItem(it.key());
        QJsonArray jsonArray = it.value().toArray();
        for (const QJsonValue &value : jsonArray) {
            QStandardItem *childItem = new QStandardItem(QString::number(value.toInt()));
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

    QMap<QString, long> portraitsMap = portraitsWidget->getItems();
    for (auto it = portraitsMap.begin(); it != portraitsMap.end(); ++it) {
        portraitIDs.append(it.value());
    }

    QMap<QString, long> classesMap = classesWidget->getItems();

    emit classifyPortraits(portraitIDs, classesMap);
    ui->loadingLabel->setMovie(movie);
    ui->loadingLabel->movie()->start();
}
