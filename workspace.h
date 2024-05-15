#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <panel.h>
#include <filesystem.h>
#include <QMessageBox>
#include <iostream>
#include "trmlshell.h"
#include <unordered_map>
#include <algorithm>
#include "ipcompare.h"

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

private:
    Panel *leftPanel;
    Panel *rightPanel;
    FileSystem *fileSystem;
    bool isLeftCurrent;
    IPCompareResults comparisonResults;
    IPCompare *ipCompare;
    QList<QWidget*> widgetsList;

private:
    void updatePanels();

    void removeFilesystemEntries(Panel* panel);
    void removeDatabaseEntries(Panel* panel);

    void copyFilesystemToDatabase(Panel* sourcePanel, Panel* destinationPanel);
    void copyFilesystemToFilesystem(Panel* sourcePanel, Panel* destinationPanel);
    void copyDatabaseToDatabase(Panel* sourcePanel, Panel* destinationPanel);
    void copyDatabaseToFilesystem(Panel* sourcePanel, Panel* destinationPanel);

    void moveFilesystemToDatabase(Panel *sourcePanel, Panel *destinationPanel);
    void moveFilesystemToFilesystem(Panel *sourcePanel, Panel *destinationPanel);
    void moveDatabaseToDatabase(Panel *sourcePanel, Panel *destinationPanel);
    void moveDatabaseToFilesystem(Panel *sourcePanel, Panel *destinationPanel);

    void createDirFileSystem(Panel *panel);
    void createDirDatabase(Panel *panel);

    void clearComparisonResults();
    void calculateComparisonParameters(const std::vector<TIPFullTermInfo *> &leftTerms,
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
    void updateInfoDB(bool isLeft, bool isPlus);
    void updateFolder(bool isLeft, QString path);
    void indexToString(bool isLeft, QModelIndex index);
    void changeSelectionMode();
    void changeCurrentPanel();
    void comparePortraits();
    void killChildren();
};

#endif // WORKSPACE_H
