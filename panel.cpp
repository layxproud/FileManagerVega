#include "panel.h"
#include <QDesktopServices>

Panel::Panel(QWidget *parent)
    : QTreeView(parent)
    , info("")
    , numberOfSelectedFolders(0)
    , numberOfSelectedFiles(0)
    , numberOfFolders(0)
    , numberOfFiles(0)
    , selectedFilesSize(0)
    , currentDirSize(0)
    , current_folder_id(0)
    , viewip{nullptr}
    , proxyModel(new PanelSortFilterProxyModel())
{
    DBmodel = new EditableNameModel(this);
    DBmodel->insertColumns(0, 8);
    QStringList Coloumns_name;
    Coloumns_name << tr("Название") << tr("ID") << tr("Размер") << tr("Дата Создания")
                  << tr("Владелец") << tr("Релевантность") << " "
                  << " ";
    int i = 0;
    foreach (QString it, Coloumns_name) {
        DBmodel->setHeaderData(i, Qt::Horizontal, QVariant(it));
        i++;
    }
    functions_of_current_BD = new TIPDBShell;
}

Panel::~Panel()
{
    delete functions_of_current_BD;
    delete proxyModel;
}

void Panel::initPanel(FileSystem *fileSystem, bool isLeft, bool isDB)
{
    proxyModel->setSourceModel(fileSystem);
    setFileSystem(proxyModel);
    this->isDB = isDB;
    this->isLeft = isLeft;

    setItemsExpandable(false);
    setRootIsDecorated(false);
    setAllColumnsShowFocus(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::NoSelection);

    connect(this, &QTreeView::clicked, this, &Panel::choose);
    connect(this, &QTreeView::doubleClicked, this, &Panel::changeDirectory);

    // Сортировка
    setSortingEnabled(true);
    sortByColumn(0, Qt::AscendingOrder);

    // Редактирование элементов
    setEditTriggers(QAbstractItemView::SelectedClicked);
    MyEditingDelegate *delegate = new MyEditingDelegate(this);
    this->setItemDelegate(delegate);
    connect(delegate, &MyEditingDelegate::editingFinished, this, &Panel::onEditFinished);
}

void Panel::populatePanel(const QString &arg, bool isDriveDatabase)
{
    if (isDriveDatabase) {
        getFunctionsDB()->Init(arg);
        setPath("/");
        setIsDB(true);
        ChangeFolderDB(1);
    } else {
        setIsDB(false);
        setFileSystem(proxyModel);
        QModelIndex sourceRootIndex = fileSystem->index(arg);
        QModelIndex proxyRootIndex = proxyModel->mapFromSource(sourceRootIndex);
        setRootIndex(proxyRootIndex);
    }
    header()->setSectionResizeMode(QHeaderView::Interactive);
    clearPanel();
    proxyModel->invalidate();
    this->setColumnWidth(0, 200);
}

void Panel::clearLists()
{
    list.clear();
    chosenFolders.clear();
    chosenItems.clear();
}

/*** ГЕТТЕРЫ И СЕТТЕРЫ ***/

void Panel::setFileSystem(QAbstractItemModel *model)
{
    this->setModel(model);
    QAbstractProxyModel *proxyModel = qobject_cast<QAbstractProxyModel *>(model);
    if (proxyModel) {
        this->fileSystem = dynamic_cast<FileSystem *>(proxyModel->sourceModel());
    } else {
        this->fileSystem = dynamic_cast<FileSystem *>(model);
    }
}

void Panel::setPath(QString path)
{
    this->path = path;
}

void Panel::setIsDB(bool isDB)
{
    this->isDB = isDB;
}

FileSystem *Panel::getFilesystem()
{
    return this->fileSystem;
}

QString Panel::getPath()
{
    return this->path;
}

bool Panel::getIsDB()
{
    return isDB;
}

QString Panel::getInfo()
{
    return this->info;
}

QModelIndexList &Panel::getList()
{
    return this->list;
}

TIPDBShell *Panel::getFunctionsDB()
{
    return functions_of_current_BD;
}

std::vector<TIPInfo *> *Panel::getItems()
{
    return &this->items;
}

std::vector<folderinfo *> *Panel::getFolders()
{
    return &this->folders;
}

QStandardItemModel *Panel::getDB()
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

std::list<TIPInfo *> &Panel::getChosenItems()
{
    return chosenItems;
}

std::list<folderinfo *> &Panel::getChosenFolders()
{
    return chosenFolders;
}

/*** ОСНОВНЫЕ ФУНКЦИИ ***/

void Panel::chooseButton()
{
    if (this->selectionMode() != QAbstractItemView::MultiSelection) {
        this->setSelectionMode(QAbstractItemView::MultiSelection);
    }
    this->setCurrentIndex(this->currentIndex());
    this->choose(this->currentIndex());
}

void Panel::choose(const QModelIndex &originalIndex)
{
    if (!originalIndex.isValid()) {
        return;
    }
    QModelIndex effectiveIndex = originalIndex;
    if (!isDB) {
        effectiveIndex = proxyModel->mapToSource(originalIndex);
    }

    // Нормализую индекс, чтобы клик по разным столбцам одной строки
    // воспринимался как клик по одному и тому же индексу
    QModelIndex normalizedIndex = effectiveIndex.sibling(effectiveIndex.row(), 0);

    if (this->selectionMode() != QAbstractItemView::MultiSelection) {
        if (this->selectionMode() == QAbstractItemView::NoSelection) {
            this->setSelectionMode(QAbstractItemView::SingleSelection);
            this->setCurrentIndex(this->currentIndex());
        }
        list.clear();
        chosenItems.clear();
        chosenFolders.clear();
        clearInfo();
        list.push_back(normalizedIndex);

        if (isDB) {
            PushDB(normalizedIndex);
        }
        emit updateInfo(isLeft, true, normalizedIndex);
    } else if (this->selectionMode() == QAbstractItemView::MultiSelection) {
        if (!list.contains(normalizedIndex)) {
            list.push_back(normalizedIndex);
            if (isDB) {
                PushDB(normalizedIndex);
            }
            emit updateInfo(isLeft, true, normalizedIndex);
        } else {
            list.removeOne(normalizedIndex);
            if (isDB) {
                RemoveDB(normalizedIndex);
            }
            emit updateInfo(isLeft, false, normalizedIndex);
        }
    }

    infoToString();
}

void Panel::changeDirectory(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    QModelIndex effectiveIndex = index;
    if (!isDB) {
        effectiveIndex = proxyModel->mapToSource(index);
    }

    if (!isDB && !fileSystem->isDir(effectiveIndex)) {
        openFile(effectiveIndex);
        return;
    }

    clearInfo();

    if (!isDB) {
        QString newPath;
        if (fileSystem->fileInfo(effectiveIndex).fileName() == "..") {
            newPath = fileSystem->filePath(fileSystem->index(fileSystem->filePath(effectiveIndex)));
        } else {
            newPath = fileSystem->filePath(effectiveIndex);
        }
        populatePanel(newPath, false);
        emit changeFolder(isLeft, newPath, isDB);
    } else {
        // Handle database directory change logic
        QStandardItem *mb_ps = this->getDB()->item(effectiveIndex.row());
        if (mb_ps->text() == "..") {
            current_folder_id = pathID.back();
            pathID.pop_back();
            this->setPath(cdUp(this->getPath()));
            this->ChangeFolderDB(this->getCurrentFolder());
        } else if (this->getPath() == "/" && DBmodel->item(effectiveIndex.row(), 2)->text() == " ") {
            pathID.push_back(current_folder_id);
            this->setCurrentFolder(this->getDB()->item(effectiveIndex.row(), 1)->text().toInt());
            path = path + this->getDB()->item(effectiveIndex.row())->text();
            this->ChangeFolderDB(this->getCurrentFolder());
        } else if (this->getPath() != "/" && DBmodel->item(effectiveIndex.row(), 2)->text() == " ") {
            pathID.push_back(current_folder_id);
            this->setCurrentFolder(this->getDB()->item(effectiveIndex.row(), 1)->text().toInt());
            path = path + "/" + this->getDB()->item(effectiveIndex.row())->text();
            this->ChangeFolderDB(this->getCurrentFolder());
        } else if (DBmodel->item(effectiveIndex.row(), 2)->text() != " ") {
            this->getFunctionsDB()->OpenItem(
                findItem(this->getDB()->item(effectiveIndex.row(), 1)->text().toInt()));
        }
        emit changeFolder(isLeft, this->getPath(), isDB);
    }

    this->clearInfo();
    this->setFocus();
    this->setSelectionMode(QAbstractItemView::NoSelection);
    this->setCurrentIndex(model()->index(0, 0, this->rootIndex()));
    this->update();
}

void Panel::changeSelectionMode()
{
    if (this->selectionMode() == QAbstractItemView::NoSelection) {
        this->setSelectionMode(QAbstractItemView::MultiSelection);
    } else if (this->selectionMode() == QAbstractItemView::MultiSelection) {
        this->setSelectionMode(QAbstractItemView::SingleSelection);
    } else if (this->selectionMode() == QAbstractItemView::SingleSelection) {
        this->setSelectionMode(QAbstractItemView::MultiSelection);
    }
    this->clearInfo();
    this->infoToString();
}

void Panel::changeCountChosenFiles(bool isPlus)
{
    if (isPlus) {
        numberOfSelectedFiles++;
    } else {
        numberOfSelectedFiles--;
    }
}

void Panel::changeCountChosenFolders(bool isPlus)
{
    if (isPlus) {
        numberOfSelectedFolders++;
    } else {
        numberOfSelectedFolders--;
    }
}

void Panel::changeSize(bool isPlus, long long int delta)
{
    if (isPlus) {
        selectedFilesSize += delta;
    } else {
        selectedFilesSize -= delta;
    }
}

void Panel::changeCurrentFolderInfo(
    QString path, long long int sizeCurrentFolder, int filesCount, int foldersCount)
{
    this->path = path;
    this->currentDirSize = sizeCurrentFolder;
    this->numberOfFiles = filesCount;
    this->numberOfFolders = foldersCount;
    this->infoToString();
    showPath(this->path);
}

void Panel::setItemData(QStandardItemModel *model, int row, int column, const QVariant &data)
{
    QStandardItem *item = new QStandardItem();
    item->setData(data, Qt::EditRole);
    model->setItem(row, column, item);
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
    DBmodel->insertRows(0, allCount);

    for (int i = 0; i < allCount; i++) {
        if (i < folders.size()) {
            setItemData(DBmodel, i, 0, folders[i]->second);
            setItemData(DBmodel, i, 1, folders[i]->first);
            setItemData(DBmodel, i, 2, " ");
            setItemData(DBmodel, i, 3, " ");
            setItemData(DBmodel, i, 4, " ");
            setItemData(DBmodel, i, 5, " ");
            setItemData(DBmodel, i, 6, " ");
            setItemData(DBmodel, i, 7, " ");
        } else {
            numberOfFiles++;
            int itemIndex = i - folders.size();
            setItemData(DBmodel, i, 0, items[itemIndex]->name);
            setItemData(DBmodel, i, 1, items[itemIndex]->id);
            setItemData(DBmodel, i, 2, items[itemIndex]->sizeInTerms);
            setItemData(DBmodel, i, 3, items[itemIndex]->creationTime);
            setItemData(DBmodel, i, 4, items[itemIndex]->ownerName);

            setItemData(
                DBmodel,
                i,
                5,
                items[itemIndex]->compData1 == 0 ? QVariant(" ")
                                                 : QVariant(items[itemIndex]->compData1));
            setItemData(
                DBmodel,
                i,
                6,
                items[itemIndex]->compData2 == 0 ? QVariant(" ")
                                                 : QVariant(items[itemIndex]->compData2));
            setItemData(
                DBmodel,
                i,
                7,
                items[itemIndex]->compData3 == 0 ? QVariant(" ")
                                                 : QVariant(items[itemIndex]->compData3));
        }

        if (DBmodel->item(i, 3)->text() == " ") {
            DBmodel->item(i, 0)->setIcon(QIcon(":/icons/resources/folder.png"));
        } else {
            DBmodel->item(i, 0)->setIcon(QIcon(":/icons/resources/dbf.png"));
        }
    }

    if (folder != 1 && folder != 0) {
        QStandardItem *back_item = new QStandardItem("..");
        DBmodel->insertRow(0, back_item);
    }

    this->setModel(DBmodel);
    this->sortByColumn(0, Qt::AscendingOrder);
    return;
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

QString Panel::cdUp(QString path)
{
    if (path == "/") {
        return path;
    }

    int last_slash = -1;
    QString buffer = path;
    for (int i = 0; i < path.size(); i++) {
        if (buffer[i] == '/') {
            last_slash = i;
        }
    }

    if (last_slash == 0) {
        buffer.remove(1, buffer.size() - 1);
    } else {
        buffer.remove(last_slash, buffer.size() - last_slash);
    }
    return buffer;
}

TIPInfo *Panel::findItem(folderid item)
{
    for (size_t i = 0; i < items.size(); i++) {
        if (items[i]->id == item) {
            return items[i];
        }
    }
    qDebug() << "ERROR";
    return nullptr;
}

folderinfo *Panel::findFolder(folderid folder)
{
    for (size_t i = 0; i < folders.size(); i++) {
        if (folders[i]->first == folder) {
            return folders[i];
        }
    }
    qDebug() << "ERROR";
    return nullptr;
}

void Panel::PushDB(QModelIndex index)
{
    if (!index.isValid()) {
        return;
    }

    if (isDB && DBmodel->item(index.row())->text() == "..") {
        return;
    } else if (isDB && DBmodel->item(index.row(), 2)->text() == " ") {
        folderinfo *fi = findFolder(DBmodel->item(index.row(), 1)->text().toInt());
        if (fi) {
            chosenFolders.push_back(fi);
            this->numberOfSelectedFolders++;
        }
    } else if (isDB && DBmodel->item(index.row(), 2)->text() != " ") {
        int itemId = DBmodel->item(index.row(), 1)->text().toInt();
        qDebug() << itemId;
        chosenItems.push_back(findItem(DBmodel->item(index.row(), 1)->text().toInt()));
        // selectedFilesSize += findItem(DBmodel->item(index.row(), 2)->text().toInt())->sizeInBytes/1024;
        selectedFilesSize += findItem(DBmodel->item(index.row(), 1)->text().toInt())->sizeInTerms;
        this->numberOfSelectedFiles++;
    }
}

void Panel::RemoveDB(QModelIndex index)
{
    if (isDB && DBmodel->item(index.row())->text() == "..") {
        return;
    } else if (isDB && DBmodel->item(index.row(), 1)->text() == " ") {
        chosenFolders.remove(findFolder(DBmodel->item(index.row(), 2)->text().toInt()));
        this->numberOfSelectedFolders--;
    } else if (isDB && DBmodel->item(index.row(), 1)->text() != " ") {
        chosenItems.remove(findItem(DBmodel->item(index.row(), 2)->text().toInt()));
        // selectedFilesSize -= findItem(DBmodel->item(index.row(), 2)->text().toInt())->sizeInBytes/1024;
        // Хотя верхняя строка корректная по отношению к размеру,
        // Я пока не замечал файлов не с нулевым размером
        selectedFilesSize -= findItem(DBmodel->item(index.row(), 2)->text().toInt())->sizeInTerms;
        this->numberOfSelectedFiles--;
    }
}

void Panel::onEditFinished(const QModelIndex &index)
{
    if (isDB && DBmodel->item(index.row())->text() == "..") {
        return;
    } else if (isDB && DBmodel->item(index.row(), 1)->text() == " ") {
        this->getFunctionsDB()->RenameFolder(
            findFolder(DBmodel->item(index.row(), 2)->text().toInt())->first,
            DBmodel->item(index.row(), 0)->text());
    } else if (isDB && DBmodel->item(index.row(), 1)->text() != " ") {
        this->getFunctionsDB()->RenameItem(
            findItem(DBmodel->item(index.row(), 2)->text().toInt()),
            DBmodel->item(index.row(), 0)->text());
    }
}

void Panel::infoToString()
{
    info.append(QString ::number(selectedFilesSize) + " KB ");
    info.append("of " + QString ::number(currentDirSize) + " KB ");
    info.append(
        "files " + QString::number(numberOfSelectedFiles) + " of " + QString::number(numberOfFiles)
        + " folders " + QString::number(numberOfSelectedFolders) + " of "
        + QString ::number(numberOfFolders));
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
    if (!currentIndex().isValid()) {
        setCurrentIndex(model()->index(0, 0, rootIndex()));
    }

    if (selectionMode() == QAbstractItemView::SingleSelection) {
        selectionModel()->clearSelection();
        clearInfo();
        infoToString();
        setSelectionMode(QAbstractItemView::NoSelection);

        if ((currentIndex().row() - 1) >= 0) {
            setCurrentIndex(model()->index(currentIndex().row() - 1, 0, currentIndex().parent()));
        } else {
            int lastRow = model()->rowCount(currentIndex().parent()) - 1;
            setCurrentIndex(model()->index(lastRow, 0, currentIndex().parent()));
        }
    } else if (selectionMode() == QAbstractItemView::NoSelection) {
        selectionModel()->clearSelection();
        getList().clear();

        if ((currentIndex().row() - 1) >= 0) {
            setCurrentIndex(model()->index(currentIndex().row() - 1, 0, currentIndex().parent()));
        } else {
            int lastRow = model()->rowCount(currentIndex().parent()) - 1;
            setCurrentIndex(model()->index(lastRow, 0, currentIndex().parent()));
        }
    } else if (selectionMode() == QAbstractItemView::MultiSelection) {
        if (currentIndex().row() - 1 >= 0) {
            setCurrentIndex(model()->index(currentIndex().row() - 1, 0, currentIndex().parent()));
            setCurrentIndex(currentIndex());
        } else {
            int lastRow = model()->rowCount(currentIndex().parent()) - 1;
            setCurrentIndex(model()->index(lastRow, 0, currentIndex().parent()));
            setCurrentIndex(currentIndex());
        }
    }
}

void Panel::arrowDown()
{
    if (!currentIndex().isValid()) {
        setCurrentIndex(model()->index(0, 0, rootIndex()));
    }

    if (selectionMode() == QAbstractItemView::SingleSelection) {
        selectionModel()->clearSelection();
        clearInfo();
        infoToString();
        setSelectionMode(QAbstractItemView::NoSelection);

        if ((currentIndex().row() + 1) < model()->rowCount(currentIndex().parent())) {
            setCurrentIndex(model()->index(currentIndex().row() + 1, 0, currentIndex().parent()));
        } else {
            setCurrentIndex(model()->index(0, 0, currentIndex().parent()));
        }
    } else if (selectionMode() == QAbstractItemView::NoSelection) {
        selectionModel()->clearSelection();
        getList().clear();

        if ((currentIndex().row() + 1) < model()->rowCount(currentIndex().parent())) {
            setCurrentIndex(model()->index(currentIndex().row() + 1, 0, currentIndex().parent()));
        } else {
            setCurrentIndex(model()->index(0, 0, currentIndex().parent()));
        }
    } else if (selectionMode() == QAbstractItemView::MultiSelection) {
        if ((currentIndex().row() + 1) < model()->rowCount(currentIndex().parent())) {
            setCurrentIndex(model()->index(currentIndex().row() + 1, 0, currentIndex().parent()));
            setCurrentIndex(currentIndex());
        } else {
            setCurrentIndex(model()->index(0, 0, currentIndex().parent()));
            setCurrentIndex(currentIndex());
        }
    }
}

void Panel::refreshDB()
{
    this->ChangeFolderDB(current_folder_id);
    this->clearInfo();
    this->infoToString();
    this->update();
    this->setSelectionMode(QAbstractItemView::NoSelection);
    this->selectionModel()->clear();
}

void Panel::openFile(const QModelIndex &index)
{
    QString filePath = fileSystem->filePath(index);
    QString extension = QFileInfo(filePath).suffix().toLower();

    if (extension == "txt" || extension == "pdf" || extension == "html" || extension == "docx") {
        QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
    } else if (extension == "xml") {
        XmlParser parser;
        XmlParser::ParsedData parsedData = parser.readFileAndParse(filePath);
        viewip = new ViewIP;
        viewip->setData(
            parsedData.id,
            parsedData.user,
            parsedData.date,
            parsedData.comment,
            parsedData.terms,
            parsedData.shingles);
        viewip->show();
    }
}
