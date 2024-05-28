#include "matchwindow.h"
#include "ui_matchwindow.h"

MatchWindow::MatchWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MatchWindow)
    , portraitsWidget(new PortraitListWidget(this))
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);
    ui->portraitsLayout->addWidget(portraitsWidget);

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

void MatchWindow::setPortraits(const QMap<QString, long> &p)
{
    portraitsWidget->setItems(p);
}

void MatchWindow::closeEvent(QCloseEvent *event)
{
    deleteLater();
    event->accept();
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
    FindMatchParams params;
    params.dbName = dbName;
    params.collectionID = ui->collectionInput->text();
    params.requestType = (ui->requestButton->isChecked()) ? "text" : "portrait";
    params.searchType = (ui->typeInput->currentIndex()) ? "min" : "max";
    params.docCount = ui->docCountInput->value();

    if (ui->requestButton->isChecked() && !ui->requestInput->text().isEmpty()) {
        params.requestText = ui->requestInput->text();
        emit findMatchingPortraits(params);
    } else if (ui->portraitsButton->isChecked() && !portraitsWidget->getItems().isEmpty()) {
        QList<int> portraitIDs;
        QMap<QString, long> portraitsMap = portraitsWidget->getItems();
        for (auto it = portraitsMap.begin(); it != portraitsMap.end(); ++it) {
            portraitIDs.append(it.value());
        }
        params.requestIDs = portraitIDs;
        emit findMatchingPortraits(params);
    } else {
        qWarning() << "Заполните все поля!";
    }
}
