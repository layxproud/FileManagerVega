#include "panel.h"
#include <QDesktopServices>
#include "CustomLineEdit.h"

Panel::Panel(QWidget *parent) :
    QTreeView(parent),
    info(""),
    numberOfSelectedFolders(0),
    numberOfSelectedFiles(0),
    numberOfFolders(0),
    numberOfFiles(0),
    selectedFilesSize(0),
    currentDirSize(0),
    current_folder_id(0)
{
    DBmodel = new EditableNameModel(this);
    DBmodel->insertColumns(0, 8);
    QStringList Coloumns_name;
    Coloumns_name << "Name" << "Size" << "Id" << "Owner" << "Date Creation" << "Relevance" << " " << " ";
    int i = 0;
    foreach (QString it, Coloumns_name)
    {
        DBmodel->setHeaderData(i, Qt::Horizontal, QVariant(it));
        i++;
    }
    functions_of_current_BD = new TIPDBShell;
}

void Panel::initPanel(FileSystem *fileSystem, bool isLeft, bool isDB)
{
    setFileSystem(fileSystem);
    this->isDB = isDB;
    this->isLeft = isLeft;

    setItemsExpandable(false);
    setRootIsDecorated(false);
    setAllColumnsShowFocus(true);
    setSelectionMode(QAbstractItemView::NoSelection);

    connect(this, &QTreeView::clicked, this, &Panel::choose);
    connect(this, &QTreeView::doubleClicked, this, &Panel::changeDirectory);

    // Сортировка
    sortByColumn(0, Qt::AscendingOrder);
    setSortingEnabled(true);

    // Редактирование элементов
    setEditTriggers(QAbstractItemView::SelectedClicked);
    MyEditingDelegate *delegate = new MyEditingDelegate(this);
    this->setItemDelegate(delegate);
    connect(delegate, &MyEditingDelegate::editingFinished, this, &Panel::onEditFinished);
}

void Panel::populatePanel(const QString &arg, bool isDriveDatabase)
{
    if (isDriveDatabase)
    {
        getFunctionsDB()->Init(arg);
        setPath("/");
        setIfDB(true);
        ChangeFolderDB(1);
        header()->setSectionResizeMode(QHeaderView::Interactive);
    }
    else
    {
        setIfDB(false);
        setFileSystem(fileSystem);
        setRootIndex(fileSystem->index(arg));
        update();
        header()->setSectionResizeMode(QHeaderView::Interactive);
    }

    clearPanel();
}

QString Panel::getPath()
{
    return this->path;
}

QString Panel::getInfo()
{
    return this->info;
}

QModelIndexList& Panel::getList()
{
    return this->list;
}

FileSystem* Panel::getFilesystem()
{
    return this->fileSystem;
}

void Panel::chooseButton()
{
    if (this->selectionMode() != QAbstractItemView::MultiSelection)
        this->setSelectionMode(QAbstractItemView::MultiSelection);
    this->setCurrentIndex(this->currentIndex());
    this->choose(this->currentIndex());
}

void Panel::choose(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    if (this->selectionMode() != QAbstractItemView::MultiSelection)
    {
        if (this->selectionMode()==QAbstractItemView::NoSelection)
        {
            this->setSelectionMode(QAbstractItemView::SingleSelection);
            this->setCurrentIndex(index);
        }
        if (!isDB)
        {
            list.clear();
            list.push_back(index);
            selectedFilesSize = 0;
            numberOfSelectedFiles = 0;
            numberOfSelectedFolders = 0;
            emit updateInfo(isLeft, true, index);
        }
        else
        {
            numberOfSelectedFiles = 0;
            numberOfSelectedFolders = 0;
            chosenItems.clear();
            chosenFolders.clear();
            PushDB(index);
            emit updateInfo(isLeft, true, index);
        }
    }
    else if (this->selectionMode() == QAbstractItemView::MultiSelection && !list.contains(index))
    {
        if (!isDB)
        {
            list.push_back(index);
            emit updateInfo(isLeft, true, index);
        }
        else
        {
            list.push_back(index);
            PushDB(index);
            emit updateInfo(isLeft, true, index);
        }
    }
    else if (this->selectionMode() == QAbstractItemView::MultiSelection && list.contains(index))
    {
        if (!isDB)
        {
            list.removeOne(index);
            emit updateInfo(isLeft, false, index);
        }
        else
        {
            list.removeOne(index);
            RemoveDB(index);
            emit updateInfo(isLeft, false, index);
        }
    }
    info.append(QString :: number(selectedFilesSize) + " KB ");
    info.append("of " + QString :: number(currentDirSize) + " KB ");
    info.append("files " + QString:: number(numberOfSelectedFiles) + " of " + QString::number(numberOfFiles) + " folders " +QString::number(numberOfSelectedFolders)+" of " + QString :: number(numberOfFolders));
    emit showInfo(info);
    info.clear();
}

void Panel::changeDirectory(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    // Check if the clicked item is a file
    if (!isDB && !fileSystem->isDir(index)) {
        QString fileName = fileSystem->fileName(index);
        QString filePath = fileSystem->filePath(index);

        QString extension = QFileInfo(filePath).suffix().toLower();

        if (extension == "txt" || extension == "pdf" || extension == "html" || extension == "docx") {
            QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
        }

        return;
    }

    selectedFilesSize = 0;
    QStandardItem *mb_ps = this->getDB()->item(index.row());
    if (isDB && mb_ps->text() == "..")
    {
        current_folder_id = pathID.back();
        pathID.pop_back();
        this->setPath(cdUp(this->getPath()));
        this->changeCurrentFolderInfo(this->getPath(), 0, 0, 0);
        this->ChangeFolderDB(this->getCurrentFolder()); // тут мы должны сменить директорию
    }
    else if (isDB && this->getPath() == "/" && DBmodel->item(index.row(), 1)->text() == " ")
    {
        pathID.push_back(current_folder_id);
        this->setCurrentFolder(this->getDB()->item(index.row(), 2)->text().toInt());
        path = path + this->getDB()->item(index.row())->text();
        this->changeCurrentFolderInfo(this->getPath(), 0, 0, 0);
        this->ChangeFolderDB(this->getCurrentFolder()); // тут мы должны сменить директорию
    }
    else if (isDB && this->getPath() != "/" && DBmodel->item(index.row(), 1)->text() == " ")
    {
        pathID.push_back(current_folder_id);
        this->setCurrentFolder(this->getDB()->item(index.row(), 2)->text().toInt());
        path = path + "/" + this->getDB()->item(index.row())->text();
        this->changeCurrentFolderInfo(this->getPath(), 0, 0, 0);
        this->ChangeFolderDB(this->getCurrentFolder()); // тут мы должны сменить директорию
    }
    else if (isDB && DBmodel->item(index.row(), 1)->text() != " ")
    {
        this->getFunctionsDB()->OpenItem(findItem(this->getDB()->item(index.row(), 2)->text().toInt()));
    }
    else
    {
        if (this->fileSystem->fileInfo(index).fileName() == "..")
        {
            this->setRootIndex(fileSystem->index(fileSystem->filePath(index)));
            this->changeFolder(isLeft, index);
        }
        else
        {
            this->setRootIndex(index);
            this->setPath(fileSystem->filePath(index));
            this->changeFolder(isLeft, index);
        }
    }
    this->clearInfo();
    this->setFocus();
    this->setSelectionMode(QAbstractItemView::NoSelection);
    this->setCurrentIndex(model()->index(0,0, this->rootIndex()));
    if (isDB)
        changeCurrentFolderInfo(path, 0,0,0);
    else
        InfoToString();
}

void Panel::changeSelectionMode()
{
    if (this->selectionMode() == QAbstractItemView::NoSelection)
    {
        this->setSelectionMode(QAbstractItemView::MultiSelection);
    }
    else if (this->selectionMode() == QAbstractItemView::MultiSelection)
    {
        this->setSelectionMode(QAbstractItemView::SingleSelection);
    }
    else if (this->selectionMode() == QAbstractItemView::SingleSelection)
    {
        this->setSelectionMode(QAbstractItemView::MultiSelection);
    }
    this->clearInfo();
    this->InfoToString();
}

void Panel::changeCountChosenFiles(bool isPlus)
{
    if (isPlus)
        numberOfSelectedFiles++;
    else
        numberOfSelectedFiles--;
}

void Panel::changeCountChosenFolders(bool isPlus)
{
    if (isPlus)
        numberOfSelectedFolders++;
    else
        numberOfSelectedFolders--;
}

void Panel::changeSize(bool isPlus, long long int delta)
{
    if (isPlus)
        selectedFilesSize += delta;
    else
        selectedFilesSize -= delta;
}

void Panel::changeCurrentFolderInfo(QString path, long long int sizeCurrentFolder, int filesCount, int foldersCount)
{
    this->path = path;
    this->currentDirSize = sizeCurrentFolder;
    this->numberOfFiles = filesCount;
    this->numberOfFolders = foldersCount;
    this->InfoToString();
    showPath(this->path);
}

void Panel::setIsleft(bool isLeft)
{
    this->isLeft = isLeft;
}

void Panel::setPath(QString path)
{
    this->path = path;
}

void Panel::ChangeFolderDB(folderid folder)
{
    items.clear();
    folders.clear();
    chosenItems.clear();
    chosenFolders.clear();

    functions_of_current_BD->GetFolderContents(folder, items, folders);
    functions_of_current_BD->ChangeFolder(folder);
    int allCount = items.size() + folders.size();
    DBmodel->removeRows(0, DBmodel->rowCount());
    this->update();
    DBmodel->insertRows(0, allCount);
    QStringList Coloumns_value;

    for (unsigned int i = 0; i < allCount; i++)
    {
        if (i < folders.size())
        {
            Coloumns_value << folders[i]->second << " "
                           << QString::number(folders[i]->first)
                           << " " << " " << " " << " " << " ";
        }
        else
        {
            numberOfFiles++;
            Coloumns_value << items[i % items.size()]->name << QString::number(items[i % items.size()]->sizeInTerms) << QString::number(items[i % items.size()]->id)  << items[i % items.size()]->ownerName << items[i % items.size()]->creationTime.toString();

            if (items[i % items.size()]->compData1 == 0)
            {
                Coloumns_value << " ";
            }
            else
            {
                Coloumns_value << QString::number(items[i % items.size()]->compData1);
            }

            if (items[i % items.size()]->compData2 == 0)
            {
                Coloumns_value << " ";
            }
            else
            {
                Coloumns_value << QString::number(items[i % items.size()]->compData2);
            }

            if (items[i% items.size()]->compData3 == 0)
            {
                Coloumns_value << " ";
            }
            else
            {
                Coloumns_value << QString::number(items[i % items.size()]->compData3);
            }
        }

        int j = 0;
        foreach (QString it, Coloumns_value)
        {
            QStandardItem *item = new QStandardItem(it);
            DBmodel->setItem(i, j, item);
            if (j == 3)
            {
                if (DBmodel->item(i, 3)->text() == " ")
                {
                    DBmodel->item(i, 0)->setIcon(QIcon("resources/folder.png"));
                }
                else
                {
                    DBmodel->item(i, 0)->setIcon(QIcon("resources/dbf.png"));
                }
            }
            j++;
        }
        Coloumns_value.clear();
    }

    if (folder != 1 && folder != 0)
    {
        QStandardItem *back_item = new QStandardItem("..");
        DBmodel->insertRow(0, back_item);
    }

    this->setModel(DBmodel);
    this->currentDirSize = 0;
    for (size_t i = 0; i < items.size(); i++)
    {
        this->currentDirSize += items[i]->sizeInBytes / 1024;
    }
    numberOfFolders = allCount - numberOfFiles;
    return;
}

void Panel::setFileSystem(FileSystem * filesystem)
{
    this->setModel(filesystem);
    this->fileSystem = filesystem;
    this->update();
}

bool Panel::getIsDB()
{
    return isDB;
}

void Panel::setIfDB(bool ifDB)
{
    this->isDB = ifDB;
}

TIPDBShell* Panel::getFunctionsDB()
{
    return functions_of_current_BD;
}

std::vector<TIPInfo*>* Panel::getItems()
{
    return &this->items;
}

std::vector<folderinfo*>* Panel::getFolders()
{
    return &this->folders;
}

void Panel::clearPanel()
{
    list.clear();
    this->setSelectionMode(QAbstractItemView::NoSelection);
    this->selectedFilesSize = 0;
    this->numberOfSelectedFiles = 0;
    this->numberOfSelectedFolders = 0;
    current_folder_id = 0;
}

QStandardItemModel* Panel::getDB()
{
    return this->DBmodel;
}

void Panel::setCurrentFolder(folderid folder)
{
    this->current_folder_id = folder;
}

folderid Panel::getCurrentFolder()
{
    return this->current_folder_id;
}

QString Panel::cdUp(QString path)
{
    if (path == "/") return path;
    else
    {
        int last_slash = -1;
        QString buffer = path;
        for (int i = 0; i < path.size(); i++)
        {
            if (buffer[i] == '/') last_slash = i;
        }
        if (last_slash == 0)
        {
            buffer.remove(1, buffer.size()-1);
        }
        else
        {
            buffer.remove(last_slash, buffer.size()-last_slash);
        }
        return buffer;
    }
}

TIPInfo* Panel::findItem(folderid item)
{
    for (size_t i = 0; i < items.size(); i++)
    {
        if (items[i]->id == item)
            return items[i];
    }
    qDebug() << "ERROR";
    return nullptr;
}

folderinfo* Panel::findFolder(folderid folder)
{
    for (size_t i = 0; i < folders.size(); i++)
    {
        if (folders[i]->first == folder)
            return folders[i];
    }
    qDebug() << "ERROR";
    return nullptr;
}

void Panel::PushDB(QModelIndex index)
{
    if (!index.isValid())
        return;

    if (isDB && DBmodel->item(index.row())->text() == "..")
    {
        return;
    }
    else if(isDB && /*this->getPath() == "/" &&*/ DBmodel->item(index.row(), 1)->text() == " ")
    {
        folderinfo *fi = findFolder(DBmodel->item(index.row(), 2)->text().toInt());
        if (fi)
        {
            chosenFolders.push_back(fi);
            this->numberOfSelectedFolders++;
        }
    }
    else if (isDB && /*this->getPath() == "/" &&*/ DBmodel->item(index.row(), 1)->text() != " ")
    {
        chosenItems.push_back(findItem(DBmodel->item(index.row(), 2)->text().toInt()));
        selectedFilesSize += findItem(DBmodel->item(index.row(), 2)->text().toInt())->sizeInBytes/1024;
        this->numberOfSelectedFiles++;
    }
}

void Panel::RemoveDB(QModelIndex index)
{
    qDebug() << "Removing item";
    if (isDB && DBmodel->item(index.row())->text() == "..")
    {
        return;
    }
    else if(isDB && this->getPath() == "/" && DBmodel->item(index.row(), 1)->text() == " ")
    {
        chosenFolders.remove(findFolder(DBmodel->item(index.row(), 2)->text().toInt()));
        this->numberOfSelectedFolders--;
    }
    else if (isDB && this->getPath() == "/" && DBmodel->item(index.row(), 1)->text() != " ")
    {
        chosenItems.remove(findItem(DBmodel->item(index.row(), 2)->text().toInt()));
        selectedFilesSize -= findItem(DBmodel->item(index.row(), 2)->text().toInt())->sizeInBytes/1024;
        this->numberOfSelectedFiles--;
    }
}

std::list <TIPInfo*> &Panel::getChosenItems()
{
    return chosenItems;
}

std::list <folderinfo*> &Panel::getChosenFolders()
{
    return chosenFolders;
}

void Panel::onEditFinished()
{
    qDebug() << "Edit finished";
}

void Panel::InfoToString()
{
    info.append(QString :: number(selectedFilesSize) + " KB ");
    info.append("of " + QString :: number(currentDirSize) + " KB ");
    info.append("files " + QString:: number(numberOfSelectedFiles) + " of " + QString::number(numberOfFiles) + " folders " +QString::number(numberOfSelectedFolders)+" of " + QString :: number(numberOfFolders));
    emit showInfo(info);
    info.clear();
}
void Panel::clearInfo()
{
    selectedFilesSize = 0;
    numberOfSelectedFiles = 0;
    numberOfSelectedFolders = 0;
    list.clear();
}

void Panel::arrowUp()
{
    if (!currentIndex().isValid())
    {
        setCurrentIndex(model()->index(0, 0, rootIndex()));
    }

    if (selectionMode() == QAbstractItemView::SingleSelection)
    {
        selectionModel()->clearSelection();
        clearInfo();
        InfoToString();
        setSelectionMode(QAbstractItemView::NoSelection);

        if ((currentIndex().row() - 1) >= 0)
        {
            setCurrentIndex(model()->index(currentIndex().row() - 1, 0, currentIndex().parent()));
        }
        else
        {
            int lastRow = model()->rowCount(currentIndex().parent()) - 1;
            setCurrentIndex(model()->index(lastRow, 0, currentIndex().parent()));
        }
    }
    else if (selectionMode() == QAbstractItemView::NoSelection)
    {
        selectionModel()->clearSelection();
        getList().clear();

        if ((currentIndex().row() - 1) >= 0)
        {
            setCurrentIndex(model()->index(currentIndex().row() - 1, 0, currentIndex().parent()));
        }
        else
        {
            int lastRow = model()->rowCount(currentIndex().parent()) - 1;
            setCurrentIndex(model()->index(lastRow, 0, currentIndex().parent()));
        }
    }
    else if (selectionMode() == QAbstractItemView::MultiSelection)
    {
        if (currentIndex().row() - 1 >= 0)
        {
            setCurrentIndex(model()->index(currentIndex().row() - 1, 0, currentIndex().parent()));
            setCurrentIndex(currentIndex());
        }
        else
        {
            int lastRow = model()->rowCount(currentIndex().parent()) - 1;
            setCurrentIndex(model()->index(lastRow, 0, currentIndex().parent()));
            setCurrentIndex(currentIndex());
        }
    }
}

void Panel::arrowDown()
{
    if (!currentIndex().isValid())
    {
        setCurrentIndex(model()->index(0, 0, rootIndex()));
    }

    if (selectionMode() == QAbstractItemView::SingleSelection)
    {
        selectionModel()->clearSelection();
        clearInfo();
        InfoToString();
        setSelectionMode(QAbstractItemView::NoSelection);

        if ((currentIndex().row() + 1) < model()->rowCount(currentIndex().parent()))
            setCurrentIndex(model()->index(currentIndex().row() + 1, 0, currentIndex().parent()));
        else
            setCurrentIndex(model()->index(0, 0, currentIndex().parent()));
    }
    else if (selectionMode() == QAbstractItemView::NoSelection)
    {
        selectionModel()->clearSelection();
        getList().clear();

        if ((currentIndex().row() + 1) < model()->rowCount(currentIndex().parent()))
            setCurrentIndex(model()->index(currentIndex().row() + 1, 0, currentIndex().parent()));
        else
            setCurrentIndex(model()->index(0, 0, currentIndex().parent()));
    }
    else if (selectionMode() == QAbstractItemView::MultiSelection)
    {
        if ((currentIndex().row() + 1) < model()->rowCount(currentIndex().parent()))
        {
            setCurrentIndex(model()->index(currentIndex().row() + 1, 0, currentIndex().parent()));
            setCurrentIndex(currentIndex());
        }
        else
        {
            setCurrentIndex(model()->index(0, 0, currentIndex().parent()));
            setCurrentIndex(currentIndex());
        }
    }
}


void Panel::refreshDB()
{
    this->ChangeFolderDB(current_folder_id);
    this->clearInfo();
    this->InfoToString();
    this->update();
    this->setSelectionMode(QAbstractItemView::NoSelection);
    this->selectionModel()->clear();
}
