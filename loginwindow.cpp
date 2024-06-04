#include "loginwindow.h"
#include "ui_loginwindow.h"
#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QMessageBox>

LoginWindow::LoginWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginWindow)
{
    ui->setupUi(this);
    movie = new QMovie(":/gifs/resources/loading.gif");
    connect(ui->loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginButtonClicked);
    ui->loginInput->installEventFilter(this);
    ui->passwordInput->installEventFilter(this);
}

LoginWindow::~LoginWindow()
{
    delete ui;
}

bool LoginWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Up) {
            if (watched == ui->passwordInput) {
                ui->loginInput->setFocus();
            }
        } else if (keyEvent->key() == Qt::Key_Down) {
            if (watched == ui->loginInput) {
                ui->passwordInput->setFocus();
            }
        }
    }
    return false;
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
    ui->loadingLabel->setMovie(movie);
    ui->loadingLabel->movie()->start();
    ui->loginInput->setEnabled(false);
    ui->passwordInput->setEnabled(false);
    emit loginAttempt("dim_nov01", "CneltynVbh'f");
}
