#ifndef PANEL_H
#define PANEL_H
#include <QFileSystemModel>
#include <QTreeView>
#include<QInputDialog>
#include<QStringList>
#include <QKeyEvent>
#include <QShortcut>
#include <QFile>
#include <QTreeWidgetItem>
#include "QStandardItemModel"
#include "tipdbshell.h"
#include <filesystem.h>
#include <iostream>
#include "sortableheaderview.h"
#include <QTimer>

class Panel : public QTreeView
{
    Q_OBJECT

public:
    Panel(QWidget *parent);

    void initPanel(FileSystem *fileSystem, bool isLeft, bool isDB);
    void populatePanel(const QString &arg, bool isDriveDatabase);

private:
    FileSystem *fileSystem;
    SortableHeaderView *headerView;
    QTimer *clickTimer;
    QModelIndex clickedIndex; // Stores the QModelIndex for the single click action
    bool isDB;
    bool isLeft;

    /* Переменные хранящие информацию о файловой системе */
    QString info;
    QString path;
    int numberOfSelectedFolders, numberOfSelectedFiles;
    int numberOfFolders, numberOfFiles;
    long long int selectedFilesSize;
    long long int currentDirSize;


    QModelIndexList list; // список выделенных индексов


    QStandardItemModel *DBmodel;
    std::vector <TIPInfo*> items;
    std::vector <folderinfo*> folders;
    std::list <TIPInfo*> chosenItems;
    std::list <folderinfo*> chosenFolders;
    unsigned int count_elements_DB;
    TIPDBShell *functions_of_current_BD;
    folderid current_folder_id;
    std::list<folderid> pathID;

signals:
    /* Обновление GUI */
    void showInfo(const QString &);
    void showPath(const QString &);

    void updateInfo(bool isLeft, bool ifPlus, QModelIndex index);
    void changeFolder(bool isLeft, QModelIndex index);
    void createDir(bool, QString);

public slots:
    QString getPath(); // получаем путь
    QString getInfo(); // получаем информацию под панелью
    QModelIndexList &getList(); // получаем список выбранных индексов
    FileSystem* getFilesystem();

    void chooseButton ();
    void choose(const QModelIndex &index); // в этом методе определяем выбран
    void changeDirectory(const QModelIndex &index); // меняем директорию на панели
    void changeSelectionMode(); //меняем режим выбора на панели(один элемент или несколько)
    void changeCountChosenFiles(bool isPlus);
    void changeCountChosenFolders(bool isPlus);
    void changeSize(bool isPlus, long long int delta);
    void changeCurrentFolderInfo(QString path, long long int sizeCurrentFolder, int filesCount, int foldersCount);
    void setIsleft(bool isLeft);
    void setPath(QString path);
    void setFileSystem(FileSystem *filesystem);
    void ChangeFolderDB(folderid folder);
    bool getIsDB();
    void setIfDB(bool ifDB);
    TIPDBShell* getFunctionsDB();
    std::vector<TIPInfo*>* getItems();
    std::vector<folderinfo*>* getFolders();
    void clearPanel();
    QStandardItemModel* getDB();
    folderid getCurrentFolder();
    void setCurrentFolder(folderid folder);
    QString cdUp(QString path);
    folderinfo* findFolder(folderid folder);
    TIPInfo* findItem(folderid folder);
    void PushDB(QModelIndex index);
    void RemoveDB(QModelIndex index);
    std::list <TIPInfo*> &getChosenItems();
    std::list <folderinfo*> &getChosenFolders();

    void InfoToString();
    void clearInfo();
    void arrowUp();
    void arrowDown();
    void refreshDB();

private slots:
    void onClicked(const QModelIndex &index);
    void onSingleClickTimeout();
};

#endif // PANEL_H
