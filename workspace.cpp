#include "workspace.h"

Workspace::Workspace(Panel *left, Panel *right, FileSystem *filesystem) :
    leftPanel{left},
    rightPanel{right},
    fileSystem{filesystem},
    isLeftCurrent{true}
{
    leftPanel->setFocus();
    leftPanel->setCurrentIndex(this->leftPanel->currentIndex());

    connect(leftPanel, &Panel::updateInfo, this, &Workspace::updateInfo);
    connect(rightPanel, &Panel::updateInfo, this, &Workspace::updateInfo);
    connect(leftPanel, &Panel::changeFolder, this, &Workspace::indexToString);
    connect(rightPanel, &Panel::changeFolder, this, &Workspace::indexToString);
}

void Workspace::choose()
{
    if (isLeftCurrent)
    {
        leftPanel->chooseButton();
    }
    else
    {
        rightPanel->chooseButton();
    }
}

bool Workspace:: getIsLeftCurrent()
{
    return isLeftCurrent;
}

void Workspace::remove()
{
    if (isLeftCurrent)
    {
        if (!leftPanel->getIfDB())
        {
            QModelIndexList selectedIndexes = leftPanel->getList();

            foreach (const QModelIndex &index, selectedIndexes)
            {
                fileSystem->removeIndex(index);
            }

            leftPanel->getList().clear();
        }
        else
        {
            for (auto i = leftPanel->getChosenItems().begin(); i != leftPanel->getChosenItems().end(); i++)
            {
                leftPanel->getFunctionsDB()->DeleteItem(*i, leftPanel->getCurrentFolder());
            }
            leftPanel->getChosenItems().clear();

            for (auto i = leftPanel->getChosenFolders().begin(); i != leftPanel->getChosenFolders().end(); i++)
            {
                leftPanel->getFunctionsDB()->DeleteFolder((*i)->first);
            }
            leftPanel->getChosenFolders().clear();
            ////////left->ChangeFolderDB(left->getCurrentFolder());
        }
    }
    else
    {
        if (!rightPanel->getIfDB())
        {
            QModelIndexList selectedIndexes = rightPanel->getList();

            foreach (const QModelIndex &index, selectedIndexes)
            {
                fileSystem->removeIndex(index);
            }

            rightPanel->getList().clear();
        }
        else
        {
            for (auto i = rightPanel->getChosenItems().begin(); i != rightPanel->getChosenItems().end(); i++)
            {
                rightPanel->getFunctionsDB()->DeleteItem(*i, rightPanel->getCurrentFolder());
            }
            rightPanel->getChosenItems().clear();

            for (auto i = rightPanel->getChosenFolders().begin(); i != rightPanel->getChosenFolders().end(); i++)
            {
                folderinfo *fi = *i;
                if (fi)
                    rightPanel->getFunctionsDB()->DeleteFolder(fi->first);
            }
            rightPanel->getChosenFolders().clear();
            ////////right->ChangeFolderDB(right->getCurrentFolder());
        }
    }
    leftPanel->setSelectionMode(QAbstractItemView::NoSelection);
    rightPanel->setSelectionMode(QAbstractItemView::NoSelection);
    leftPanel->update();
    rightPanel->update();
    if (leftPanel->getIfDB())
        leftPanel->refreshDB();
    if (rightPanel->getIfDB())
        rightPanel->refreshDB();
}

void Workspace::copy()
{
    if (isLeftCurrent)
    {
        if (!leftPanel->getIfDB())
        {
            if(rightPanel->getIfDB())
            {
                QModelIndexList il = leftPanel->getList();
                for(auto i = il.begin(); i != il.end(); i++)
                {
                    rightPanel->getFunctionsDB()->CopyFileToDB(leftPanel->getFilesystem()->fileInfo(*i).absoluteFilePath(), rightPanel->getCurrentFolder());
                }
            }
            else
            {
                QModelIndexList il = leftPanel->getList();
                for(QModelIndexList::iterator i = il.begin(); i != il.end(); i++)
                {
                    leftPanel->getFilesystem()->copyIndex(*i, rightPanel->getPath());
                }
            }
        }
        else
        {
            if (rightPanel->getIfDB())
            {
                if (leftPanel->getFunctionsDB()->getDataBase()->databaseName() != rightPanel->getFunctionsDB()->getDataBase()->databaseName())
                {
                    for (auto i = leftPanel->getChosenItems().begin(); i != leftPanel->getChosenItems().end(); i++)
                    {
                        leftPanel->getFunctionsDB()->CopyItemBetweenDatabases(*i,
                                                                              leftPanel->getFunctionsDB()->getDataBase(),
                                                                              rightPanel->getFunctionsDB()->getDataBase(),
                                                                              rightPanel->getCurrentFolder());
                    }
                    for (auto i = leftPanel->getChosenFolders().begin(); i != leftPanel->getChosenFolders().end(); i++)
                    {
                        leftPanel->getFunctionsDB()->CopyFolder((*i)->first, leftPanel->getCurrentFolder(), rightPanel->getCurrentFolder());
                    }
                }
                else
                {
                    for (auto i = leftPanel->getChosenItems().begin(); i != leftPanel->getChosenItems().end(); i++)
                    {
                        leftPanel->getFunctionsDB()->CopyItemWithinDatabase(*i, leftPanel->getCurrentFolder(), rightPanel->getCurrentFolder());
                    }
                    for (auto i = leftPanel->getChosenFolders().begin(); i != leftPanel->getChosenFolders().end(); i++)
                    {
                        leftPanel->getFunctionsDB()->CopyFolder((*i)->first, leftPanel->getCurrentFolder(), rightPanel->getCurrentFolder());
                    }
                }
            }
            else
            {
                for (auto i = leftPanel->getChosenItems().begin(); i != leftPanel->getChosenItems().end(); i++)
                {
                    QString string = rightPanel->getPath();
                    string.append("/");
                    string.append((*i)->name.replace(':', '.').replace('/', '.').replace('\\', '.'));
                    string.append(".xml");
                    leftPanel->getFunctionsDB()->CopyItemToFile(*i, string);
                }
                for (auto i = leftPanel->getChosenFolders().begin(); i != leftPanel->getChosenFolders().end(); i++)
                {
                    leftPanel->getFunctionsDB()->CopyFolder((*i)->first, leftPanel->getCurrentFolder(), rightPanel->getCurrentFolder());//тут мы должны рекурсивно копировать папки из БД в Файловую систему, но такой функции в TIPDBSHELL нет
                }
            }
        }
    }
    else
    {
        if (!rightPanel->getIfDB())
        {

            if(leftPanel->getIfDB())
            {
                QModelIndexList il = rightPanel->getList();
                for(auto i = il.begin(); i != il.end(); i++)
                {
                    rightPanel->getFunctionsDB()->CopyFileToDB(fileSystem->fileInfo(*i).absoluteFilePath(), leftPanel->getCurrentFolder());
                }
            }
            else
            {
                QModelIndexList il = rightPanel->getList();
                for(auto i = il.begin(); i != il.end(); i++)
                {
                    rightPanel->getFilesystem()->copyIndex(*i, leftPanel->getPath());
                }
            }
        }
        else
        {
            if (leftPanel->getIfDB())
            {
                for (auto i = rightPanel->getChosenItems().begin(); i != rightPanel->getChosenItems().end(); i++)
                {
                    rightPanel->getFunctionsDB()->CopyItemWithinDatabase(*i, rightPanel->getCurrentFolder(), leftPanel->getCurrentFolder());
                }
                for (auto i = rightPanel->getChosenFolders().begin(); i != rightPanel->getChosenFolders().end(); i++)
                {
                    rightPanel->getFunctionsDB()->CopyFolder((*i)->first, rightPanel->getCurrentFolder(), leftPanel->getCurrentFolder());
                }
            }
            else
            {
                for (auto i = rightPanel->getChosenItems().begin(); i != rightPanel->getChosenItems().end(); i++)
                {
                    QString string = leftPanel->getPath();
                    string.append("/");
                    string.append((*i)->name.replace(':', '.').replace('/', '.').replace('\\', '.'));
                    string.append(".xml");
                    rightPanel->getFunctionsDB()->CopyItemToFile(*i, string);
                }
                for (auto i = rightPanel->getChosenFolders().begin(); i != rightPanel->getChosenFolders().end(); i++)
                {
                    rightPanel->getFunctionsDB()->CopyFolder((*i)->first, rightPanel->getCurrentFolder(), leftPanel->getCurrentFolder());//тут мы должны копировать папки из БД в Файловую систему, но такой функции в TIPDBSHELL нет
                }
            }
        }
    }
    leftPanel->update();
    rightPanel->update();
    if (leftPanel->getIfDB())
        leftPanel->refreshDB();
    if (rightPanel->getIfDB())
        rightPanel->refreshDB();
}
void Workspace::move()
{
    if (isLeftCurrent)
    {
        if (!leftPanel->getIfDB())
        {

            if(rightPanel->getIfDB())
            {
                for(auto i = leftPanel->getList().begin(); i != leftPanel->getList().end(); i++)
                {
                    leftPanel->getFunctionsDB()->CopyFileToDB(leftPanel->getFilesystem()->fileInfo(*i).absoluteFilePath(), rightPanel->getCurrentFolder());
                    leftPanel->getFilesystem()->removeIndex(*i);
                }
                leftPanel->getList().clear();
            }
            else
            {
                for(auto i = leftPanel->getList().begin(); i != leftPanel->getList().end(); i++)
                {
                    leftPanel->getFilesystem()->copyIndex(*i, rightPanel->getPath());
                    leftPanel->getFilesystem()->removeIndex(*i);
                }
            }
        }
        else
        {
            if (rightPanel->getIfDB())
            {
                for (auto i = leftPanel->getChosenItems().begin(); i != leftPanel->getChosenItems().end(); i++)
                {
                    leftPanel->getFunctionsDB()->CopyItemWithinDatabase(*i, leftPanel->getCurrentFolder(), rightPanel->getCurrentFolder());
                    leftPanel->getFunctionsDB()->DeleteItem(*i, leftPanel->getCurrentFolder());
                }
                for (auto i = leftPanel->getChosenFolders().begin(); i != leftPanel->getChosenFolders().end(); i++)
                {
                    leftPanel->getFunctionsDB()->CopyFolder((*i)->first, leftPanel->getCurrentFolder(), rightPanel->getCurrentFolder());
                    leftPanel->getFunctionsDB()->DeleteFolder((*i)->first);
                }
            }
            else
            {
                for (auto i = leftPanel->getChosenItems().begin(); i != leftPanel->getChosenItems().end(); i++)
                {
                    QString string = rightPanel->getPath();
                    string.append("/");
                    string.append((*i)->name.replace(':', '.').replace('/', '.').replace('\\', '.'));
                    string.append(".xml");
                    leftPanel->getFunctionsDB()->CopyItemToFile(*i, string);
                    leftPanel->getFunctionsDB()->DeleteItem(*i, leftPanel->getCurrentFolder());
                }
                for (auto i = leftPanel->getChosenFolders().begin(); i != leftPanel->getChosenFolders().end(); i++)
                {
                    leftPanel->getFunctionsDB()->CopyFolder((*i)->first, leftPanel->getCurrentFolder(), rightPanel->getCurrentFolder());//тут мы должны копировать папки из БД в Файловую систему, но такой функции в TIPDBSHELL нет
                    leftPanel->getFunctionsDB()->DeleteFolder((*i)->first);
                }
            }
        }
    }
    else
    {
        if (!rightPanel->getIfDB())
        {

            if(leftPanel->getIfDB())
            {
                for(auto i = rightPanel->getList().begin(); i != rightPanel->getList().end(); i++)
                {
                    rightPanel->getFunctionsDB()->CopyFileToDB(rightPanel->getFilesystem()->fileInfo(*i).absoluteFilePath(), leftPanel->getCurrentFolder());
                    rightPanel->getFilesystem()->removeIndex(*i);
                }
            }
            else
            {
                for(auto i = rightPanel->getList().begin(); i != rightPanel->getList().end(); i++)
                {
                    rightPanel->getFilesystem()->copyIndex(*i, leftPanel->getPath());
                    rightPanel->getFilesystem()->removeIndex(*i);
                }
            }
        }
        else
        {
            if (leftPanel->getIfDB())
            {
                for (auto i = rightPanel->getChosenItems().begin(); i != rightPanel->getChosenItems().end(); i++)
                {
                    rightPanel->getFunctionsDB()->CopyItemWithinDatabase(*i, rightPanel->getCurrentFolder(), leftPanel->getCurrentFolder());
                    rightPanel->getFunctionsDB()->DeleteItem(*i, rightPanel->getCurrentFolder());
                }
                for (auto i = rightPanel->getChosenFolders().begin(); i != rightPanel->getChosenFolders().end(); i++)
                {
                    rightPanel->getFunctionsDB()->CopyFolder((*i)->first, rightPanel->getCurrentFolder(), leftPanel->getCurrentFolder());
                    rightPanel->getFunctionsDB()->DeleteFolder((*i)->first);
                }
            }
            else
            {
                for (auto i = rightPanel->getChosenItems().begin(); i != rightPanel->getChosenItems().end(); i++)
                {
                    QString string = leftPanel->getPath();
                    string.append("/");
                    string.append((*i)->name.replace(':', '.').replace('/', '.').replace('\\', '.'));
                    string.append(".xml");
                    rightPanel->getFunctionsDB()->CopyItemToFile(*i, string);
                    rightPanel->getFunctionsDB()->DeleteItem(*i, rightPanel->getCurrentFolder());
                }
                for (auto i = rightPanel->getChosenFolders().begin(); i != rightPanel->getChosenFolders().end(); i++)
                {
                    rightPanel->getFunctionsDB()->CopyFolder((*i)->first, rightPanel->getCurrentFolder(), leftPanel->getCurrentFolder());//тут мы должны копировать папки из БД в Файловую систему, но такой функции в TIPDBSHELL нет
                    rightPanel->getFunctionsDB()->DeleteFolder((*i)->first);
                }
            }
        }
    }
    leftPanel->setSelectionMode(QAbstractItemView::NoSelection);
    rightPanel->setSelectionMode(QAbstractItemView::NoSelection);
    leftPanel->changeDirectory(leftPanel->rootIndex());
    rightPanel->changeDirectory(rightPanel->rootIndex());
    leftPanel->update();
    rightPanel->update();
    if (leftPanel->getIfDB())
        leftPanel->refreshDB();
    if (rightPanel->getIfDB())
        rightPanel->refreshDB();
}

void Workspace::updateInfo(bool isLeft, bool isPlus, QModelIndex index)
{
    if (fileSystem->fileName(index) == "..")
        return;
    if (isLeft)
    {
        isLeftCurrent = true;
        if (!leftPanel->getIfDB())
        {
            if (fileSystem->isDir(index))
            {
                leftPanel->changeSize(isPlus, 0);
                leftPanel->changeCountChosenFolders(isPlus);
            }
            else
            {
                leftPanel->changeSize(isPlus, fileSystem->size(index) / 1024);
                leftPanel->changeCountChosenFiles(isPlus);
            }
        }
        else
        {
            //С помощью функций tipdbshell получаем информацию об индексе и обновляем информацию.
            return;
        }
    }
    else
    {
        isLeftCurrent = false;
        if (!rightPanel->getIfDB())
        {
            if (fileSystem->isDir(index))
            {
                rightPanel->changeSize(isPlus, 0);
                rightPanel->changeCountChosenFolders(isPlus);
            }
            else
            {
                rightPanel->changeSize(isPlus, fileSystem->size(index) / 1024);
                rightPanel->changeCountChosenFiles(isPlus);
            }
        }
        else
        {
            //С помощью функций tipdbshell получаем информацию об индексе и обновляем информацию.
            return;
        }
    }
}

void Workspace::indexToString(bool isLeft, QModelIndex index)
{
    if (isLeft && leftPanel->getIfDB())
    {
        return;
    }
    else if (!isLeft && rightPanel->getIfDB())
    {
        return;
    }
    else
    {
        updateFolder(isLeft, fileSystem->fileInfo(index).absoluteFilePath());
    }
}

void Workspace::updateFolder(bool isLeft, QString path)
{
    QDir file;
    file.setPath(path);
    QStringList files = file.entryList(QDir::Files);
    int countFolders = 0;
    int countFiles = 0;
    long long int size = 0;
    foreach (QFileInfo i_file, file.entryInfoList())
    {
         QString i_filename(i_file.fileName());
         if (i_filename == "." || i_filename == ".." || i_filename.isEmpty())
           continue;
         if (i_file.isDir())
         {
             countFolders++;
         }
         else
         {
             countFiles++;
             size += i_file.size() / 1024;
         }
    }
    if (isLeft)
    {
         leftPanel->changeCurrentFolderInfo(path, size, countFiles, countFolders);
    }
    else
    {
         rightPanel->changeCurrentFolderInfo(path, size, countFiles, countFolders);
    }
}

void Workspace::changeSelectionMode()
{
    if (isLeftCurrent)
    {
        leftPanel->selectionModel()->clear();
        leftPanel->changeSelectionMode();
    }
    else
    {
        rightPanel->selectionModel()->clear();
        rightPanel->changeSelectionMode();
    }
}

void Workspace::changeCurrentPanel()
{
    if (isLeftCurrent)
    {
        isLeftCurrent = false;
        rightPanel->setFocus();
        if (!rightPanel->currentIndex().isValid())
             rightPanel->setCurrentIndex(rightPanel->model()->index(0, 0, rightPanel->rootIndex()));
    }
    else
    {
        isLeftCurrent = true;
        leftPanel->setFocus();
        if (!leftPanel->currentIndex().isValid())
             leftPanel->setCurrentIndex(leftPanel->model()->index(0, 0, leftPanel->rootIndex()));
    }
}

void Workspace::createDir()
{
    if (isLeftCurrent && !leftPanel->getIfDB())
    {
        QString newCatalog = QInputDialog::getText(0,
                                                   tr("Новая папка"),
                                                   tr("Введите название: "),
                                                   QLineEdit::Normal,
                                                   "");
        fileSystem->mkdir(leftPanel->rootIndex(), newCatalog);
    }
    else if (!isLeftCurrent && !rightPanel->getIfDB())
    {
        QString newCatalog = QInputDialog::getText(0,
                                                   tr("Новая папка"),
                                                   tr("Введите название: "),
                                                   QLineEdit::Normal,
                                                   "");
        fileSystem->mkdir(rightPanel->rootIndex(), newCatalog);
    }
    else if (isLeftCurrent && leftPanel->getIfDB())
    {
        QString newCatalog = QInputDialog::getText(0,
                                                   tr("Новая папка"),
                                                   tr("Введите название: "),
                                                   QLineEdit::Normal,
                                                   "");
        int new_id = leftPanel->getFunctionsDB()->NewFolder(newCatalog);
        QStandardItem *id = new QStandardItem(QString::number(new_id));
        QStandardItem *name = new QStandardItem(newCatalog);
        name->setIcon(QIcon("resources/folder.png"));
        leftPanel->getDB()->setItem(leftPanel->getFolders()->size() + leftPanel->getItems()->size(), 2, id);
        leftPanel->getDB()->setItem(leftPanel->getFolders()->size() + leftPanel->getItems()->size(), 0, name);
        folderinfo *fold = new folderinfo;
        fold->first = new_id;
        fold->second = newCatalog;
        leftPanel->getFolders()->push_back(fold);
    }
    else if (!isLeftCurrent && rightPanel->getIfDB())
    {
        QString newCatalog = QInputDialog::getText(0,
                                                   tr("Новая папка"),
                                                   tr("Введите название: "),
                                                   QLineEdit::Normal,
                                                   "");
        int new_id = rightPanel->getFunctionsDB()->NewFolder(newCatalog);
        QStandardItem *id = new QStandardItem(QString::number(new_id));
        QStandardItem *name = new QStandardItem(newCatalog);
        name->setIcon(QIcon("resources/folder.png"));
        rightPanel->getDB()->setItem(rightPanel->getFolders()->size() + rightPanel->getItems()->size(), 2, id);
        rightPanel->getDB()->setItem(rightPanel->getFolders()->size() + rightPanel->getItems()->size(), 0, name);
        folderinfo *fold = new folderinfo;
        fold->first = new_id;
        fold->second = newCatalog;
        rightPanel->getFolders()->push_back(fold);
    }

    leftPanel->update();
    rightPanel->update();
    if (leftPanel->getIfDB())
        leftPanel->refreshDB();
    if (rightPanel->getIfDB())
        rightPanel->refreshDB();
}

void Workspace::changeDir()
{
    if(isLeftCurrent)
    {
        leftPanel->changeDirectory(leftPanel->currentIndex());
        leftPanel->setFocus();
        leftPanel->setCurrentIndex(leftPanel->model()->index(0, 0, leftPanel->rootIndex()));
    }
    else
    {
        rightPanel->changeDirectory(rightPanel->currentIndex());
        rightPanel->setFocus();
    }
}

void Workspace::updateInfoDB(bool isLeft, bool isPlus)
{
    if (isLeft)
    {
        isLeftCurrent = true;
        for (auto i = leftPanel->getChosenItems().begin(); i != leftPanel->getChosenItems().end(); i++)
        {
             leftPanel->changeSize(isPlus, (*i)->sizeInBytes / 1024);
             leftPanel->changeCountChosenFiles(isPlus);
        }
        for (auto i = leftPanel->getChosenFolders().begin(); i != leftPanel->getChosenFolders().end(); i++)
        {
             leftPanel->changeSize(isPlus, 0);
             leftPanel->changeCountChosenFolders(isPlus);
        }
    }
    else
    {
        isLeftCurrent = false;
        for (auto i = rightPanel->getChosenItems().begin(); i != rightPanel->getChosenItems().end(); i++)
        {
             rightPanel->changeSize(isPlus, (*i)->sizeInBytes / 1024);
             rightPanel->changeCountChosenFiles(isPlus);
        }
        for (auto i = rightPanel->getChosenFolders().begin(); i != rightPanel->getChosenFolders().end(); i++)
        {
             rightPanel->changeSize(isPlus, 0);
             rightPanel->changeCountChosenFolders(isPlus);
        }
    }
}
