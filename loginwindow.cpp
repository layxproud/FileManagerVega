#include "loginwindow.h"
#include "ui_loginwindow.h"
#include <QDebug>
#include <QMessageBox>

LoginWindow::LoginWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginWindow)
{
    ui->setupUi(this);
    movie = new QMovie(":/gifs/resources/loading.gif");
    connect(ui->loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginButtonClicked);
}

LoginWindow::~LoginWindow()
{
    delete ui;
}

void LoginWindow::onTokenReceived()
{
    ui->loadingLabel->movie()->stop();
    ui->loadingLabel->clear();

    QMessageBox msgBox;
    msgBox.setText("Вход выполнен успешно");
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);

    connect(&msgBox, &QMessageBox::buttonClicked, this, &LoginWindow::onMsgBoxAccepted);
    connect(&msgBox, &QMessageBox::finished, this, &LoginWindow::onMsgBoxAccepted);

    msgBox.exec();
}

void LoginWindow::onLoginButtonClicked()
{
    ui->loadingLabel->setMovie(movie);
    ui->loadingLabel->movie()->start();
    emit loginAttempt(ui->loginInput->text(), ui->passwordInput->text());
}

void LoginWindow::onMsgBoxAccepted()
{
    this->accept();
}
