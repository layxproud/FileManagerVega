#include "loginwindow.h"
#include "ui_loginwindow.h"
#include <QDebug>
#include <QMessageBox>

LoginWindow::LoginWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginWindow)
{
    ui->setupUi(this);
    movie = new QMovie(":/gifs/resources/loading.gif");
    connect(ui->loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginButtonClicked);
}

LoginWindow::~LoginWindow()
{
    delete ui;
}

void LoginWindow::onTokenReceived(bool success)
{
    ui->loadingLabel->movie()->stop();
    ui->loadingLabel->clear();

    if (success) {
        this->close();
    }

    ui->loginInput->setEnabled(true);
    ui->passwordInput->setEnabled(true);
    ui->loadingLabel->setText("Вход не удался");
    ui->loadingLabel->setStyleSheet("QLabel { font-size: 20px; color : red; }");
}

void LoginWindow::onLoginButtonClicked()
{
    ui->loadingLabel->setMovie(movie);
    ui->loadingLabel->movie()->start();
    ui->loginInput->setEnabled(false);
    ui->passwordInput->setEnabled(false);
    emit loginAttempt(ui->loginInput->text(), ui->passwordInput->text());
}

void LoginWindow::onMsgBoxAccepted()
{
    this->accept();
}

void LoginWindow::on_pushButton_clicked()
{
    this->accept();
}
