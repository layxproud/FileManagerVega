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
}
