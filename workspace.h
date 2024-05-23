#ifndef WORKSPACE_H
#define WORKSPACE_H

#include "indexwindow.h"
#include "ipcompare.h"
#include "servicehandler.h"
#include <filesystem.h>
#include <iostream>
#include <panel.h>
#include <QMessageBox>

enum Button {
    Copy = 0,
    Move,
    MkDir,
    Remove,
};

class Workspace : public QObject
{
    Q_OBJECT

public:
    Workspace(Panel *left, Panel *right, FileSystem *filesystem, QObject *parent = nullptr);
    ~Workspace();

    void setServiceHandler(ServiceHandler *sh);

private:
    Panel *leftPanel;
    Panel *rightPanel;
    FileSystem *fileSystem;
    ServiceHandler *serviceHandler;
    bool isLeftCurrent;
    IPCompare *ipCompare;
    IndexWindow *indexWindow;
    QList<QWidget *> widgetsList;

private:
    void updatePanels();

    void removeFilesystemEntries(Panel *panel);
    void removeDatabaseEntries(Panel *panel);

    void copyFilesystemToDatabase(Panel *sourcePanel, Panel *destinationPanel);
    void copyFilesystemToFilesystem(Panel *sourcePanel, Panel *destinationPanel);
    void copyDatabaseToDatabase(Panel *sourcePanel, Panel *destinationPanel);
    void copyDatabaseToFilesystem(Panel *sourcePanel, Panel *destinationPanel);

    void moveFilesystemToDatabase(Panel *sourcePanel, Panel *destinationPanel);
    void moveFilesystemToFilesystem(Panel *sourcePanel, Panel *destinationPanel);
    void moveDatabaseToDatabase(Panel *sourcePanel, Panel *destinationPanel);
    void moveDatabaseToFilesystem(Panel *sourcePanel, Panel *destinationPanel);

    void createDirFileSystem(Panel *panel);
    void createDirDatabase(Panel *panel);

    void clearComparisonResults();
    void calculateComparisonParameters(
        const std::vector<TIPFullTermInfo *> &leftTerms,
        const std::vector<TIPFullTermInfo *> &rightTerms);
    void calculateComparisonCircles();
    void handleWidgetDestroyed(QObject *object);

public slots:
    bool getIsLeftCurrent();
    void choose();
    void remove();
    void copy();
    void move();
    void createDir();
    void changeDir();
    void updateInfo(bool isLeft, bool isPlus, QModelIndex index);
    void updateFolder(bool isLeft, QString path, bool isDB);
    void changeSelectionMode();
    void changeCurrentPanel();

    // Семантические функции
    void comparePortraits();
    void getXmlFile();

    // GUI
    void killChildren();
};

#endif // WORKSPACE_H
