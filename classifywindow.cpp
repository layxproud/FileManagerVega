#include "classifywindow.h"
#include "ui_classifywindow.h"
#include <QDebug>
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>

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
    connect(ui->exportButton, &QPushButton::clicked, this, &ClassifyWindow::exportDataToFile);
}

ClassifyWindow::~ClassifyWindow()
{
    delete resultView;
    delete ui;
}

void ClassifyWindow::setPortraits(const QMap<long, QString> &p)
{
    QMap<long, QString> currentClasses = classesWidget->getPortraits();
    QMap<long, QString> newUniquePortraits;

    for (auto it = p.begin(); it != p.end(); ++it) {
        if (!currentClasses.contains(it.key())) {
            newUniquePortraits.insert(it.key(), it.value());
        } else {
            QMessageBox::warning(
                this,
                tr("Попытка добавить дубликат"),
                QString(tr("Портрет %1 уже добавлен в список классов")).arg(it.value()));
        }
    }

    portraitsWidget->setPortraits(newUniquePortraits);
}

void ClassifyWindow::setClasses(const QMap<long, QString> &c)
{
    QMap<long, QString> currentPortraits = portraitsWidget->getPortraits();
    QMap<long, QString> newUniqueClasses;

    for (auto it = c.begin(); it != c.end(); ++it) {
        if (!currentPortraits.contains(it.key())) {
            newUniqueClasses.insert(it.key(), it.value());
        } else {
            QMessageBox::warning(
                this,
                tr("Попытка добавить дубликат"),
                QString(tr("Портрет %1 уже добавлен в список портретов")).arg(it.value()));
        }
    }

    classesWidget->setPortraits(newUniqueClasses);
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
    QMap<long, QString> portraitsMap = portraitsWidget->getPortraits();

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

void ClassifyWindow::writeItem(QTextStream &out, QStandardItem *item, int level)
{
    out << QString(level * 3, ' ') << item->text() << "\n";

    for (int i = 0; i < item->rowCount(); ++i) {
        writeItem(out, item->child(i), level + 1);
    }
}

void ClassifyWindow::exportDataToFile()
{
    QString fileName
        = QFileDialog::getSaveFileName(this, tr("Экспорт результата"), "", tr("Text Files (*.txt)"));
    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось открыть файл"));
        return;
    }

    QTextStream out(&file);

    QStandardItemModel *model = static_cast<QStandardItemModel *>(resultView->model());
    if (!model) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Модель не найдена"));
        return;
    }

    for (int i = 0; i < model->rowCount(); ++i) {
        writeItem(out, model->item(i), 0);
        out << "\n";
    }

    file.close();
}

void ClassifyWindow::onApplyButtonClicked()
{
    if (portraitsWidget->getPortraits().isEmpty() || classesWidget->getPortraits().isEmpty()) {
        qWarning() << "Не выбрано ни одного класса или портрета";
        return;
    }

    QList<long> portraitIDs;

    QMap<long, QString> portraitsMap = portraitsWidget->getPortraits();
    for (auto it = portraitsMap.begin(); it != portraitsMap.end(); ++it) {
        portraitIDs.append(it.key());
    }

    QMap<long, QString> classesMap = classesWidget->getPortraits();

    emit classifyPortraits(portraitIDs, classesMap);
    ui->loadingLabel->setMovie(movie);
    ui->loadingLabel->movie()->start();
}
