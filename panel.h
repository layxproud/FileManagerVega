#ifndef PANEL_H
#define PANEL_H

#include "QStandardItemModel"
#include "tipdbshell.h"
#include "viewip.h"
#include "xmlparser.h"
#include <filesystem.h>
#include <iostream>
#include <QFile>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QKeyEvent>
#include <QShortcut>
#include <QSortFilterProxyModel>
#include <QStringList>
#include <QStyledItemDelegate>
#include <QTreeView>
#include <QTreeWidgetItem>

class EditableNameModel : public QStandardItemModel
{
    Q_OBJECT

public:
    EditableNameModel(QObject *parent = nullptr)
        : QStandardItemModel(parent)
    {}

    Qt::ItemFlags flags(const QModelIndex &index) const override
    {
        Qt::ItemFlags defaultFlags = QStandardItemModel::flags(index);

        // Можно редактировать только столбец с именем
        if (index.column() == 0) {
            return defaultFlags | Qt::ItemIsEditable;
        } else {
            return defaultFlags & ~Qt::ItemIsEditable;
        }
    }

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override
    {
        QList<QStandardItem *> backItem;
        for (int i = 0; i < rowCount(); ++i) {
            if (item(i, 0) && item(i, 0)->text() == "..") {
                backItem.append(takeRow(i));
                break;
            }
        }

        QStandardItemModel::sort(column, order);

        if (!backItem.isEmpty()) {
            insertRow(0, backItem);
        }
    }
};

class MyEditingDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    MyEditingDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {}

    void setModelData(
        QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override
    {
        QStyledItemDelegate::setModelData(editor, model, index);
        emit editingFinished(index);
    }

signals:
    void editingFinished(const QModelIndex &) const;
};

class PanelSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    PanelSortFilterProxyModel(QObject *parent = nullptr)
        : QSortFilterProxyModel(parent)
    {}

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override
    {
        if (sortColumn() == 0) {
            QFileSystemModel *fsm = qobject_cast<QFileSystemModel *>(sourceModel());
            bool asc = sortOrder() == Qt::AscendingOrder ? true : false;

            QFileInfo leftFileInfo = fsm->fileInfo(left);
            QFileInfo rightFileInfo = fsm->fileInfo(right);

            // If DotAndDot move in the beginning
            if (sourceModel()->data(left).toString() == "..")
                return asc;
            if (sourceModel()->data(right).toString() == "..")
                return !asc;

            // Move dirs upper
            if (!leftFileInfo.isDir() && rightFileInfo.isDir()) {
                return !asc;
            }
            if (leftFileInfo.isDir() && !rightFileInfo.isDir()) {
                return asc;
            }
        }

        return QSortFilterProxyModel::lessThan(left, right);
    }
};

class Panel : public QTreeView
{
    Q_OBJECT

public:
    explicit Panel(QWidget *parent);
    ~Panel();

    void initPanel(FileSystem *fileSystem, bool isLeft, bool isDB);
    void populatePanel(const QString &arg, bool isDriveDatabase);
    void setIsDB(bool isDB);
    void setPath(QString path);
    bool getIsDB();

private:
    bool isDB;
    bool isLeft;
    PanelSortFilterProxyModel *proxyModel;

    /* Файловая система */
    FileSystem *fileSystem;
    QString info;
    QString path;
    int numberOfSelectedFolders, numberOfSelectedFiles;
    int numberOfFolders, numberOfFiles;
    long long int selectedFilesSize;
    long long int currentDirSize;
    QModelIndexList list;

    /* База данных */
    QStandardItemModel *DBmodel;
    std::vector<TIPInfo *> items;
    std::vector<folderinfo *> folders;
    std::list<TIPInfo *> chosenItems;
    std::list<folderinfo *> chosenFolders;
    unsigned int count_elements_DB;
    TIPDBShell *functions_of_current_BD;
    folderid current_folder_id;
    std::list<folderid> pathID;

    // Окно для отображения XML файлов
    ViewIP *viewip;

signals:
    /* Обновление GUI */
    void showInfo(const QString &);
    void showPath(const QString &);

    void updateInfo(bool isLeft, bool isPlus, QModelIndex index);
    void changeFolder(bool isLeft, QString path, bool isDB);

public slots:
    QString getPath();
    QString getInfo();
    QModelIndexList &getList();
    FileSystem *getFilesystem();

    void chooseButton();
    void choose(const QModelIndex &index);
    void changeDirectory(const QModelIndex &index);
    void changeSelectionMode();
    void changeCountChosenFiles(bool isPlus);
    void changeCountChosenFolders(bool isPlus);
    void changeSize(bool isPlus, long long int delta);
    void changeCurrentFolderInfo(
        QString path, long long int sizeCurrentFolder, int filesCount, int foldersCount);

    void setFileSystem(QAbstractItemModel *model);
    void ChangeFolderDB(folderid folder);

    TIPDBShell *getFunctionsDB();
    std::vector<TIPInfo *> *getItems();
    std::vector<folderinfo *> *getFolders();
    void clearPanel();
    QStandardItemModel *getDB();
    folderid getCurrentFolder();
    void setCurrentFolder(folderid folder);
    QString cdUp(QString path);
    folderinfo *findFolder(folderid folder);
    TIPInfo *findItem(folderid folder);
    void PushDB(QModelIndex index);
    void RemoveDB(QModelIndex index);
    std::list<TIPInfo *> &getChosenItems();
    std::list<folderinfo *> &getChosenFolders();
    void onEditFinished(const QModelIndex &index);

    void infoToString();
    void clearInfo();
    void arrowUp();
    void arrowDown();
    void refreshDB();

private slots:
    void openFile(const QModelIndex &index);
};

#endif // PANEL_H
