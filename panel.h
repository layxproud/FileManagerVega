#ifndef PANEL_H
#define PANEL_H

#include <QFileSystemModel>
#include <QTreeView>
#include <QInputDialog>
#include <QStringList>
#include <QKeyEvent>
#include <QShortcut>
#include <QFile>
#include <QTreeWidgetItem>
#include "QStandardItemModel"
#include "tipdbshell.h"
#include <filesystem.h>
#include <iostream>
#include <QHeaderView>
#include <QStyledItemDelegate>
#include "xmlparser.h"
#include "viewip.h"

/**
 * @brief Класс для модели, в которой редактируется только первый столбец
 */
class EditableNameModel : public QStandardItemModel {
    Q_OBJECT

public:
    /**
     * @brief Базовый конструтор
     * @param parent - родитель
     */
    EditableNameModel(QObject *parent = nullptr) : QStandardItemModel(parent) {}

    /**
     * @brief Задание флагов для столбцов
     * @param index - индекс столбца
     * @return
     */
    Qt::ItemFlags flags(const QModelIndex &index) const override {
        Qt::ItemFlags defaultFlags = QStandardItemModel::flags(index);

        // Можно редактировать только столбец с именем
        if (index.column() == 0) {
            return defaultFlags | Qt::ItemIsEditable;
        } else {
            return defaultFlags & ~Qt::ItemIsEditable;
        }
    }
};

/**
 * @brief Класс для корректного редактирования объектов БД
 */
class MyEditingDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    /**
     * @brief Стандартный конструтор
     * @param parent - родитель
     */
    MyEditingDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    /**
     * @brief Возвращает обновленные данные в модель
     */
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override {
        QStyledItemDelegate::setModelData(editor, model, index);
        emit editingFinished(index);
    }

signals:
    /**
     * @brief По завершении редактирования отправляет сигнал
     */
    void editingFinished(const QModelIndex &) const;
};

/**
 * @brief The Panel class
 */
class Panel : public QTreeView
{
    Q_OBJECT

public:
    explicit Panel(QWidget *parent);

    void initPanel(FileSystem *fileSystem, bool isLeft, bool isDB);
    void populatePanel(const QString &arg, bool isDriveDatabase);

private:
    bool isDB;
    bool isLeft;

    /* Файловая система */
    FileSystem *fileSystem;
    QString info;
    QString path;
    int numberOfSelectedFolders, numberOfSelectedFiles;
    int numberOfFolders, numberOfFiles;
    long long int selectedFilesSize;
    long long int currentDirSize;
    QModelIndexList list; // список выделенных индексов

    /* База данных */
    QStandardItemModel *DBmodel;
    std::vector <TIPInfo*> items;
    std::vector <folderinfo*> folders;
    std::list <TIPInfo*> chosenItems;
    std::list <folderinfo*> chosenFolders;
    unsigned int count_elements_DB;
    TIPDBShell *functions_of_current_BD;
    folderid current_folder_id;
    std::list<folderid> pathID;

    // Окно для отображения XML файлов
    ViewIP* viewip;

signals:
    /* Обновление GUI */
    void showInfo(const QString &);
    void showPath(const QString &);

    void updateInfo(bool isLeft, bool isPlus, QModelIndex index);
    void changeFolder(bool isLeft, QModelIndex index);

public slots:
    /**
     * @brief Функция возвращающая текущий путь
     * @return
     */
    QString getPath(); // получаем путь
    QString getInfo(); // получаем информацию под панелью
    QModelIndexList &getList(); // получаем список выбранных индексов
    FileSystem* getFilesystem();

    void chooseButton();
    /**
     * @brief Определяет действие при одиночном клике
     * @param index - индекс нажатого элемента
     */
    void choose(const QModelIndex &index);
    /**
     * @brief Определяет действие при двойном клике
     * @param index - индекс нажатого элемента
     */
    void changeDirectory(const QModelIndex &index);
    /**
     * @brief Меняет режим выбора элементов
     */
    void changeSelectionMode();
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
    void onEditFinished(const QModelIndex &index);

    void InfoToString();
    void clearInfo();
    void arrowUp();
    void arrowDown();
    void refreshDB();
};

#endif // PANEL_H
