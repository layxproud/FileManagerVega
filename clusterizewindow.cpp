#include "clusterizewindow.h"
#include "ui_clusterizewindow.h"
#include <QDebug>
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>

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
    connect(ui->exportButton, &QPushButton::clicked, this, &ClusterizeWindow::exportDataToFile);
}

ClusterizeWindow::~ClusterizeWindow()
{
    delete resultView;
    delete ui;
}

void ClusterizeWindow::setPortraits(const QMap<long, QString> &p)
{
    portraitsWidget->setPortraits(p);
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
        QMessageBox::warning(this, tr("Ошибка"), res);
    } else {
        ui->loadingLabel->setText("");

        QJsonDocument jsonResponse = QJsonDocument::fromJson(res.toUtf8());
        if (jsonResponse.isNull() || !jsonResponse.isObject()) {
            qCritical() << "Ошибка при разборе JSON ответа";
            QMessageBox::warning(this, tr("Ошибка"), tr("Ошибка при разборе JSON ответа"));
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
    QMap<long, QString> portraitsMap = portraitsWidget->getPortraits();

    for (auto clusterIt = jsonObject.begin(); clusterIt != jsonObject.end(); ++clusterIt) {
        QStandardItem *clusterItem = new QStandardItem(clusterIt.key());

        QJsonObject clusterObject = clusterIt.value().toObject();

        // IDs
        QStandardItem *idsItem = new QStandardItem("Портреты");
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
        QStandardItem *termsItem = new QStandardItem("Топ 10 терминов");
        QJsonArray termsArray = clusterObject["ten_terms"].toArray();
        for (const QJsonValue &termValue : termsArray) {
            QStandardItem *childItem = new QStandardItem(termValue.toString());
            termsItem->appendRow(childItem);
        }
        clusterItem->appendRow(termsItem);

        model->appendRow(clusterItem);
    }
}

void ClusterizeWindow::writeItem(QTextStream &out, QStandardItem *item, int level)
{
    out << QString(level * 3, ' ') << item->text() << "\n";

    for (int i = 0; i < item->rowCount(); ++i) {
        writeItem(out, item->child(i), level + 1);
    }
}

void ClusterizeWindow::exportDataToFile()
{
    QString fileName
        = QFileDialog::getSaveFileName(this, tr("Экспорт результата"), "", tr("Text Files (*.txt)"));
    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"), tr("Unable to open file for writing"));
        return;
    }

    QTextStream out(&file);

    QStandardItemModel *model = static_cast<QStandardItemModel *>(resultView->model());
    if (!model) {
        QMessageBox::warning(this, tr("Error"), tr("No model found"));
        return;
    }

    for (int i = 0; i < model->rowCount(); ++i) {
        writeItem(out, model->item(i), 0);
        out << "\n";
    }

    file.close();
}

void ClusterizeWindow::onApplyButtonClicked()
{
    if (portraitsWidget->getPortraits().isEmpty()) {
        qWarning() << "Не выбрано ни одного портрета";
        return;
    }

    QList<long> portraitIDs;
    QMap<long, QString> portraitsMap = portraitsWidget->getPortraits();
    for (auto it = portraitsMap.begin(); it != portraitsMap.end(); ++it) {
        portraitIDs.append(it.key());
    }

    emit clusterizePortraits(portraitIDs, ui->clustersNumInput->value());
    ui->loadingLabel->setMovie(movie);
    ui->loadingLabel->movie()->start();
}
