#include "clusterizewindow.h"
#include "ui_clusterizewindow.h"
#include <QDebug>
#include <QHeaderView>

ClusterizeWindow::ClusterizeWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ClusterizeWindow)
    , portraitsWidget(new PortraitListWidget(this))
    , resultView(new QTreeView())
{
    ui->setupUi(this);
    ui->portraitsLayout->addWidget(portraitsWidget);

    movie = new QMovie(":/gifs/resources/loading.gif");

    connect(ui->cancelButton, &QPushButton::clicked, this, &ClusterizeWindow::close);
    connect(ui->applyButton, &QPushButton::clicked, this, &ClusterizeWindow::onApplyButtonClicked);
    connect(
        portraitsWidget,
        &PortraitListWidget::addPortraitsSignal,
        this,
        &ClusterizeWindow::addPortraitsSignal);
}

ClusterizeWindow::~ClusterizeWindow()
{
    delete resultView;
    delete ui;
}

void ClusterizeWindow::setPortraits(const QMap<long, QString> &p)
{
    portraitsWidget->setItems(p);
}

void ClusterizeWindow::setDbName(const QString &name)
{
    dbName = name;
    ui->dbNameInput->setText(dbName);
}

void ClusterizeWindow::closeEvent(QCloseEvent *event)
{
    deleteLater();

    event->accept();
}

void ClusterizeWindow::onClusterizationComplete(bool success, const QString &res)
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
        model->setHorizontalHeaderLabels({"Кластеризация"});

        populateModel(model, jsonObject);

        // Clear the layout if resultView is already added
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

void ClusterizeWindow::populateModel(QStandardItemModel *model, const QJsonObject &jsonObject)
{
    QMap<long, QString> portraitsMap = portraitsWidget->getItems();

    for (auto clusterIt = jsonObject.begin(); clusterIt != jsonObject.end(); ++clusterIt) {
        QStandardItem *clusterItem = new QStandardItem(clusterIt.key());

        QJsonObject clusterObject = clusterIt.value().toObject();

        // IDs
        QStandardItem *idsItem = new QStandardItem("IDs");
        QJsonArray idsArray = clusterObject["ids"].toArray();
        for (const QJsonValue &idValue : idsArray) {
            long id = idValue.toInt();
            QString displayValue = portraitsMap.contains(id) ? portraitsMap.value(id)
                                                             : QString::number(id);
            QStandardItem *childItem = new QStandardItem(displayValue);
            idsItem->appendRow(childItem);
        }
        clusterItem->appendRow(idsItem);

        // ten_terms
        QStandardItem *termsItem = new QStandardItem("Terms");
        QJsonArray termsArray = clusterObject["ten_terms"].toArray();
        for (const QJsonValue &termValue : termsArray) {
            QStandardItem *childItem = new QStandardItem(termValue.toString());
            termsItem->appendRow(childItem);
        }
        clusterItem->appendRow(termsItem);

        model->appendRow(clusterItem);
    }
}

void ClusterizeWindow::onApplyButtonClicked()
{
    if (portraitsWidget->getItems().isEmpty()) {
        qWarning() << "Не выбрано ни одного портрета";
        return;
    }

    QList<long> portraitIDs;
    QMap<long, QString> portraitsMap = portraitsWidget->getItems();
    for (auto it = portraitsMap.begin(); it != portraitsMap.end(); ++it) {
        portraitIDs.append(it.key());
    }

    emit clusterizePortraits(portraitIDs, ui->clustersNumInput->value());
    ui->loadingLabel->setMovie(movie);
    ui->loadingLabel->movie()->start();
}
