#include "matchlevelwindow.h"
#include "ui_matchlevelwindow.h"
#include <QDebug>

MatchLevelWindow::MatchLevelWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MatchLevelWindow)
{
    ui->setupUi(this);

    connect(ui->requestDocButton, &QRadioButton::toggled, this, &MatchLevelWindow::onRadioButton);
    connect(ui->requestTextButton, &QRadioButton::toggled, this, &MatchLevelWindow::onRadioButton);
    connect(ui->addButton, &QPushButton::clicked, this, &MatchLevelWindow::addPortraitsSignal);
    connect(ui->removeButton, &QPushButton::clicked, this, &MatchLevelWindow::onRemoveButton);
    connect(ui->cancelButton, &QPushButton::clicked, this, &MatchLevelWindow::close);
    connect(ui->applyButton, &QPushButton::clicked, this, &MatchLevelWindow::onApplyButton);
}

MatchLevelWindow::~MatchLevelWindow()
{
    delete ui;
}

void MatchLevelWindow::setPortrait(const QString &name, long id)
{
    inputPortraitName = name;
    inputPortraitID = id;
    ui->docName_1->setText(inputPortraitName + " ID: " + QString::number(inputPortraitID));
}

void MatchLevelWindow::setMatchPortrait(const QString &name, long id)
{
    matchPortraitName = name;
    matchPortraitID = id;
    ui->docName_2->setText(matchPortraitName + " ID: " + QString::number(matchPortraitID));
}

void MatchLevelWindow::closeEvent(QCloseEvent *event)
{
    deleteLater();

    event->accept();
}

void MatchLevelWindow::onRadioButton()
{
    if (ui->requestDocButton->isChecked()) {
        ui->stackedWidget->setCurrentIndex(1);
        matchPortraitName = "";
        matchPortraitID = 0;
        ui->docName_2->setText("");
    } else {
        ui->stackedWidget->setCurrentIndex(0);
        requestText = "";
        ui->requestInput->setText("");
    }
}

void MatchLevelWindow::onRemoveButton()
{
    matchPortraitName = "";
    matchPortraitID = 0;
    ui->docName_2->setText("");
}

void MatchLevelWindow::onApplyButton()
{
    if (ui->requestTextButton->isChecked() && !ui->requestInput->text().isEmpty()) {
        emit findMatchLevelSignal(dbName, inputPortraitID, QString("text"), ui->requestInput->text());
    } else if (ui->requestDocButton->isChecked() && !ui->docName_2->text().isEmpty()) {
        emit findMatchLevelSignal(
            dbName, inputPortraitID, QString("portrait"), QString::number(matchPortraitID));
    } else {
        qWarning() << "Заполните все поля!";
    }
}
