#include "clusterizewindow.h"
#include "ui_clusterizewindow.h"
#include <QDebug>

ClusterizeWindow::ClusterizeWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ClusterizeWindow)
    , portraitsWidget(new PortraitListWidget(this))
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
    delete ui;
}

void ClusterizeWindow::setPortraits(const QMap<QString, long> &p)
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
        browser = new QTextBrowser(this);
        ui->rightLayout->addWidget(browser);
        browser->setText(res);
    }
}

void ClusterizeWindow::onApplyButtonClicked()
{
    if (portraitsWidget->getItems().isEmpty()) {
        qWarning() << "Не выбрано ни одного портрета";
        return;
    }

    QList<long> portraitIDs;
    QMap<QString, long> portraitsMap = portraitsWidget->getItems();
    for (auto it = portraitsMap.begin(); it != portraitsMap.end(); ++it) {
        portraitIDs.append(it.value());
    }

    emit clusterizePortraits(portraitIDs, ui->clustersNumInput->value());
    ui->loadingLabel->setMovie(movie);
    ui->loadingLabel->movie()->start();
}
