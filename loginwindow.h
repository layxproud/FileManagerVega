#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QDialog>
#include <QMovie>

namespace Ui {
class LoginWindow;
}

class LoginWindow : public QDialog
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

private:
    Ui::LoginWindow *ui;
    QMovie *movie;

public slots:
    void onTokenReceived(bool success);

private slots:
    void onLoginButtonClicked();
    void onMsgBoxAccepted();

    void on_pushButton_clicked();

signals:
    void loginAttempt(const QString &login, const QString &pass);
};

#endif // LOGINWINDOW_H
