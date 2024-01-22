#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "workspace.h"
#include "panel.h"
#include "filesystem.h"
#include "inifile.h"
#include <QSettings>
#include "tipdbshell.h"
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

private:
    Ui::MainWindow *ui;
    Workspace *workspace;
    FileSystem *fileSystem;
    TIniFile *iniFile;

    QShortcut *tab;
    QShortcut *backspace;
    QShortcut *insert;
    QShortcut *enter;
    QShortcut *up;
    QShortcut *down;
    QSettings *settings;

private:
    void initDrivesComboBoxes();
    void initShortcuts();
    void initButtons();

    void setPathLabels(QLabel *label, const QString& arg, bool isDriveDatabase);

private slots:
    void onDriveChanged(const QString &arg);
};

#endif // MAINWINDOW_H
