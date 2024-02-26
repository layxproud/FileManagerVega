#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <panel.h>
#include <filesystem.h>
#include <QMessageBox>
#include <iostream>

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
    Workspace(Panel *left, Panel *right, FileSystem *filesystem);

private:
    Panel *leftPanel; // левая панель
    Panel *rightPanel;// правая панель
    FileSystem *fileSystem; //файловая система
    bool isLeftCurrent; // с помощью этой переменной определяем текущую панель

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

public slots:
    bool getIsLeftCurrent();
    void choose();
    void remove();
    void copy();
    void move();
    void updateInfo(bool isLeft, bool isPlus, QModelIndex index);
    void updateInfoDB(bool isLeft, bool isPlus);
    void updateFolder(bool isLeft, QString path);
    void indexToString(bool isLeft, QModelIndex index);
    void changeSelectionMode();
    void changeCurrentPanel();
    void createDir();
    void changeDir();
};

#endif // WORKSPACE_H
