#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "filesystem.h"
#include "inifile.h"
#include "loginwindow.h"
#include "servicehandler.h"
#include "workspace.h"
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::MainWindow *ui;
    Workspace *workspace;
    FileSystem *fileSystem;
    TIniFile *iniFile;
    LoginWindow *loginWindow;
    ServiceHandler *serviceHandler;

    QShortcut *tab;
    QShortcut *backspace;
    QShortcut *insert;
    QShortcut *enter;
    QShortcut *up;
    QShortcut *down;

private slots:
    void initDrivesComboBoxes();
    void initShortcuts();
    void initButtons();
    void initToolbar();
    void setPathLabels(QLabel *label, const QString &arg, bool isDriveDatabase);
    void onDriveChanged(const QString &arg);
};

#endif // MAINWINDOW_H
