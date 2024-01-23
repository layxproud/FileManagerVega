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

public slots:
    bool getIsLeftCurrent(); // получаем информацию о текущей панели
    void choose(); // выбор элемента
    void remove(); // удаление элемента
    void copy(); // копирование элемента
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
