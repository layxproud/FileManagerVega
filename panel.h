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

        // Allow editing only for the name column
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
        QModelIndex normalizedLeft = left.sibling(left.row(), 0);
        QModelIndex normalizedRight = right.sibling(right.row(), 0);

        QFileSystemModel *fsm = qobject_cast<QFileSystemModel *>(sourceModel());
        if (!fsm)
            return QSortFilterProxyModel::lessThan(left, right);

        if (sourceModel()->data(normalizedLeft).toString() == "..")
            return sortOrder() == Qt::AscendingOrder;
        if (sourceModel()->data(normalizedRight).toString() == "..")
            return sortOrder() != Qt::AscendingOrder;

        int col = sortColumn();
        if (col == 0 || col == 2) {
            QString leftData = sourceModel()->data(left).toString().toLower();
            QString rightData = sourceModel()->data(right).toString().toLower();
            return leftData < rightData;
        }

        if (col == 1) {
            QFileInfo fileInfoLeft(fsm->filePath(left));
            QFileInfo fileInfoRight(fsm->filePath(right));
            auto leftSize = fileInfoLeft.size();
            auto rightSize = fileInfoRight.size();

            return leftSize < rightSize;
        }

        if (col == 3) {
            QFileInfo fileInfoLeft(fsm->filePath(left));
            QFileInfo fileInfoRight(fsm->filePath(right));
            QDateTime leftDate = fileInfoLeft.lastModified();
            QDateTime rightDate = fileInfoRight.lastModified();

            return leftDate < rightDate;
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
    void clearLists();
    FileSystem *getFilesystem();
    QStandardItemModel *getDB();
    QString getPath();
    bool getIsDB();
    QModelIndexList &getList();
    TIPDBShell *getFunctionsDB();
    std::vector<TIPInfo *> *getItems();
    std::vector<folderinfo *> *getFolders();
    folderid getCurrentFolder();
    std::list<TIPInfo *> &getChosenItems();
    std::list<folderinfo *> &getChosenFolders();
    void refreshFS();
    void refreshDB();

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

    void setFileSystem(QAbstractItemModel *model);
    void setPath(QString path);
    void setIsDB(bool isDB);
    void setCurrentFolder(folderid folder);
    void ChangeFolderDB(folderid folder);
    void setItemData(QStandardItemModel *model, int row, int column, const QVariant &data);
    void clearPanel();
    QString cdUp(QString path);
    void infoToString();
    void clearInfo();
    folderinfo *findFolder(folderid folder);
    TIPInfo *findItem(folderid folder);
    void PushDB(QModelIndex index);
    void RemoveDB(QModelIndex index);

signals:
    /* Обновление GUI */
    void showInfo(const QString &);
    void showPath(const QString &);

    void updateInfo(bool isLeft, bool isPlus, QModelIndex index);
    void changeFolder(bool isLeft, QString path, bool isDB);

public slots:
    void chooseButton();
    void changeDirectory(const QModelIndex &index);
    void changeSelectionMode();
    void changeCountChosenFiles(bool isPlus);
    void changeCountChosenFolders(bool isPlus);
    void changeSize(bool isPlus, long long int delta);
    void changeCurrentFolderInfo(
        QString path, long long int sizeCurrentFolder, int filesCount, int foldersCount);

private slots:
    void handleSingleClick(const QModelIndex &index);
    void choose(const QItemSelection &selected, const QItemSelection &deselected);
    void openFile(const QModelIndex &index);
    void onEditFinished(const QModelIndex &index);
};

#endif // PANEL_H
