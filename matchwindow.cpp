#include "matchwindow.h"
#include "ui_matchwindow.h"
#include <QHeaderView>

MatchWindow::MatchWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MatchWindow)
    , portraitsWidget(new PortraitListWidget(this))
    , resultView(new QTreeView())
    , model(new QStandardItemModel(this))
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);
    ui->portraitsLayout->addWidget(portraitsWidget);

    movie = new QMovie(":/gifs/resources/loading.gif");

    connect(ui->requestButton, &QRadioButton::toggled, this, &MatchWindow::onRadioButton);
    connect(ui->portraitsButton, &QRadioButton::toggled, this, &MatchWindow::onRadioButton);
    connect(ui->cancelButton, &QPushButton::clicked, this, &MatchWindow::close);
    connect(ui->applyButton, &QPushButton::clicked, this, &MatchWindow::onApplyButton);
    connect(
        portraitsWidget,
        &PortraitListWidget::addPortraitsSignal,
        this,
        &MatchWindow::addPortraitsSignal);
}

MatchWindow::~MatchWindow()
{
    delete ui;
}

void MatchWindow::setPortraits(const QMap<long, QString> &p)
{
    portraitsWidget->setPortraits(p);
}

void MatchWindow::onFindMatchingComplete(bool success, const QString &res)
{
    ui->loadingLabel->movie()->stop();
    ui->loadingLabel->clear();

    if (!success) {
        ui->loadingLabel->setText(res);
        ui->loadingLabel->setStyleSheet("QLabel { font-size: 14px; color : red; }");
    } else {
        ui->loadingLabel->setText("");

        QJsonDocument jsonResponse = QJsonDocument::fromJson(res.toUtf8());
        if (jsonResponse.isNull()) {
            qCritical() << "Получен пустой JSON";
            ui->loadingLabel->setText("Получен пустой JSON");
            ui->loadingLabel->setStyleSheet("QLabel { font-size: 14px; color : red; }");
            return;
        }

        if (jsonResponse.isArray()) {
            QJsonArray jsonArray = jsonResponse.array();
            populateModel(jsonArray);
        } else {
            qCritical() << "Непредвиденный формат ответа";
            ui->loadingLabel->setText("Непредвиденный формат ответа");
            ui->loadingLabel->setStyleSheet("QLabel { font-size: 14px; color : red; }");
            return;
        }

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

void MatchWindow::closeEvent(QCloseEvent *event)
{
    deleteLater();
    event->accept();
}

void MatchWindow::populateModel(const QJsonArray &jsonArray)
{
    model->clear();
    model->setHorizontalHeaderLabels({"Найденные портреты"});

    for (const QJsonValue &value : jsonArray) {
        long id = value.toInt();
        QStandardItem *item = new QStandardItem(QString::number(id));
        model->appendRow(item);
    }
}

void MatchWindow::onRadioButton()
{
    if (ui->requestButton->isChecked()) {
        ui->stackedWidget->setCurrentIndex(0);
        portraitsWidget->clear();
    } else {
        ui->stackedWidget->setCurrentIndex(1);
        ui->requestInput->setText("");
    }
}

void MatchWindow::onApplyButton()
{
    if (ui->collectionInput->text().isEmpty()) {
        qWarning() << "Выберите коллекцию для поиска";
        return;
    }

    FindMatchParams params;
    params.dbName = dbName;
    params.collectionID = ui->collectionInput->text();
    params.requestType = (ui->requestButton->isChecked()) ? "text" : "portrait";
    params.searchType = (ui->typeInput->currentIndex()) ? "min" : "max";
    params.docCount = ui->docCountInput->value();

    if (ui->requestButton->isChecked() && !ui->requestInput->text().isEmpty()) {
        params.requestText = ui->requestInput->text();
        emit findMatchingPortraits(params);
    } else if (ui->portraitsButton->isChecked() && !portraitsWidget->getPortraits().isEmpty()) {
        QList<int> portraitIDs;
        QMap<long, QString> portraitsMap = portraitsWidget->getPortraits();
        for (auto it = portraitsMap.begin(); it != portraitsMap.end(); ++it) {
            portraitIDs.append(it.key());
        }
        params.requestIDs = portraitIDs;
        emit findMatchingPortraits(params);
    } else {
        qWarning() << "Заполните все поля!";
    }
    ui->loadingLabel->setMovie(movie);
    ui->loadingLabel->movie()->start();
}
