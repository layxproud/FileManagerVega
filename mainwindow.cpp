#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow{parent},
    ui{new Ui::MainWindow},
    fileSystem{new FileSystem(this)},
    iniFile{new TIniFile("db.ini")}
{
    ui->setupUi(this);

    ui->leftPanel->initPanel(fileSystem, true, false);
    ui->rightPanel->initPanel(fileSystem, false, false);
    connect(ui->leftPanel, &Panel::showInfo, ui->leftInfo, &QLabel::setText);
    connect(ui->rightPanel, &Panel::showInfo, ui->rightInfo, &QLabel::setText);
    connect(ui->leftPanel, &Panel::showPath, ui->labelLeftPath, &QLabel::setText);
    connect(ui->rightPanel, &Panel::showPath, ui->labelRightPath, &QLabel::setText);

    workspace = new Workspace(ui->leftPanel, ui->rightPanel, fileSystem);
    workspace->updateFolder(true, QDir::drives().at(0).path());
    workspace->updateFolder(false, QDir::drives().at(0).path());

    initDrivesComboBoxes();
    initShortcuts();
    initButtons();
}

MainWindow::~MainWindow()
{
    delete workspace;
    delete iniFile;
    delete ui;
}

void MainWindow::initDrivesComboBoxes()
{
    connect(ui->leftBox, &QComboBox::currentTextChanged, this, &MainWindow::onDriveChanged);
    connect(ui->rightBox, &QComboBox::currentTextChanged, this, &MainWindow::onDriveChanged);

    foreach (const auto &drive, QDir::drives())
    {
        ui->leftBox->addItem(drive.filePath());
        ui->rightBox->addItem(drive.filePath());
    }

    for (int i = 0; i < iniFile->GetSectionCount(); i++)
    {
        ui->leftBox->addItem(QString::fromStdString(iniFile->GetSections()[i]));
        ui->rightBox->addItem(QString::fromStdString(iniFile->GetSections()[i]));
    }
}

void MainWindow::initShortcuts()
{
    tab = new QShortcut(this);
    tab->setKey(Qt::Key_Tab);
    connect(tab, &QShortcut::activated, workspace, &Workspace::changeCurrentPanel);

    backspace = new QShortcut(this);
    backspace->setKey(Qt::Key_Backspace);
    connect(backspace, &QShortcut::activated, workspace, &Workspace::changeSelectionMode);

    insert = new QShortcut(this);
    insert->setKey(Qt::Key_Insert);
    connect(insert, &QShortcut::activated, workspace, &Workspace::choose);

    enter = new QShortcut(this);
    enter->setKey(Qt::Key_Return);
    connect(enter, &QShortcut::activated, workspace, &Workspace::changeDir);

    up = new QShortcut(this);
    up->setKey(Qt::Key_Up);
    connect(up, &QShortcut::activated, ui->leftPanel, &Panel::arrowUp);
    connect(up, &QShortcut::activated, ui->rightPanel, &Panel::arrowUp);

    down = new QShortcut(this);
    down->setKey(Qt::Key_Down);
    connect(down, &QShortcut::activated, ui->leftPanel, &Panel::arrowDown);
    connect(down, &QShortcut::activated, ui->rightPanel, &Panel::arrowDown);
}

void MainWindow::initButtons()
{
    ui->viewButton->setShortcut(Qt::Key_F3);
    connect(ui->viewButton, &QPushButton::clicked, workspace, &Workspace::changeDir);

    ui->copyButton->setShortcut(Qt::Key_F5);
    connect(ui->copyButton, &QPushButton::clicked, workspace, &Workspace::copy);

    ui->moveButton->setShortcut(Qt::Key_F6);
    connect(ui->moveButton, &QPushButton::clicked, workspace, &Workspace::move);

    ui->createButton->setShortcut(Qt::Key_F7);
    connect(ui->createButton, &QPushButton::clicked, workspace, &Workspace::createDir);

    ui->deleteButton->setShortcut(Qt::Key_F8);
    connect(ui->deleteButton, &QPushButton::clicked, workspace, &Workspace::remove);

    ui->sortButton->setShortcut(Qt::Key_F12);
    // TODO: IMPLEMENT SORT BUTTON

    connect(ui->exitButton, &QPushButton::clicked, this, &MainWindow::close);
}

void MainWindow::setPathLabels(QLabel *label, const QString &arg, bool isDriveDatabase)
{
    if (isDriveDatabase)
        label->setText("/");
    else
        label->setText(arg);
}

void MainWindow::onDriveChanged(const QString &arg)
{
    bool isDriveDatabase = false;
    for (int i = 0; i < iniFile->GetSectionCount(); i++)
    {
        if (arg.toStdString() == iniFile->GetSections()[i])
        {
            isDriveDatabase = true;
            break;
        }
    }

    QComboBox *comboBox = qobject_cast<QComboBox*>(sender());

    if (comboBox == ui->leftBox)
    {
        ui->leftPanel->populatePanel(arg, isDriveDatabase);
        setPathLabels(ui->labelLeftPath, arg, isDriveDatabase);
        workspace->updateFolder(true, ui->labelLeftPath->text());
    }
    else if (comboBox == ui->rightBox)
    {
        ui->rightPanel->populatePanel(arg, isDriveDatabase);
        setPathLabels(ui->labelRightPath, arg, isDriveDatabase);
        workspace->updateFolder(false, ui->labelRightPath->text());
    }
}
