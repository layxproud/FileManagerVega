#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "workspace.h"
#include "panel.h"
#include "filesystem.h"
#include "inifile.h"
#include <QSettings>
#include "tipdbshell.h"
#include "vector"
#include <QHeaderView>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_leftBox_activated(const QString &arg1);

    void on_rightBox_activated(const QString &arg1);

    void on_exitButton_clicked();

private:
    Ui::MainWindow *ui;
    Workspace *workspace;
    int leftCountChosenFiles, leftCountChosenFolders;
    int rightCountChosenFiles, rightCountChosenFolders;
    int long long leftSize, rightSize;
    FileSystem *filesystem;
    QShortcut *tab;
    QShortcut *backspace;
    QShortcut *insert;
    QShortcut *enter;
    QShortcut *up;
    QShortcut *down;
    QSettings *settings;
    TIniFile *inifile;
};

#endif // MAINWINDOW_H
