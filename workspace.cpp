#include "workspace.h"

Workspace::Workspace(Panel *left, Panel *right, FileSystem *filesystem) :
    isLeftCurrent{true}
{
    this->left = left;
    this->right = right;
    this->filesystem = filesystem;
    this->left->setFocus();
    this->left->setCurrentIndex(this->left->currentIndex());

    connect(left, &Panel::updateInfo, this, &Workspace::updateInfo);
    connect(right, &Panel::updateInfo, this, &Workspace::updateInfo);
    connect(left, &Panel::changeFolder, this, &Workspace::indexToString);
    connect(right, &Panel::changeFolder, this, &Workspace::indexToString);

}

void Workspace::choose()
{
    if (isLeftCurrent)
    {
        left->chooseButton();
    }
    else
    {
        right->chooseButton();
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
        if (!left->getIfDB())
        {
            for(auto i = left->getList().begin(); i != left->getList().end(); i++)
            {
                filesystem->removeIndex(*i);
            }
            left->getList().clear();
        }
        else
        {
            for (auto i = left->getChosenItems().begin(); i != left->getChosenItems().end(); i++)
            {
                left->getFunctionsDB()->DeleteItem(*i, left->getCurrentFolder());
            }
            left->getChosenItems().clear();
            for (auto i = left->getChosenFolders().begin(); i != left->getChosenFolders().end(); i++)
            {
                left->getFunctionsDB()->DeleteFolder((*i)->first);
            }
            left->getChosenFolders().clear();
            ////////left->ChangeFolderDB(left->getCurrentFolder());
        }
    }
    else
    {
        if (!right->getIfDB())
        {
            for(auto i = right->getList().begin(); i != right->getList().end(); i++)
            {
                right->getFilesystem()->removeIndex(*i);
            }
            right->getList().clear();
        }
        else
        {
            for (auto i = right->getChosenItems().begin(); i != right->getChosenItems().end(); i++)
            {
                right->getFunctionsDB()->DeleteItem(*i, right->getCurrentFolder());
            }
            right->getChosenItems().clear();

            for (auto i = right->getChosenFolders().begin(); i != right->getChosenFolders().end(); i++)
            {
                folderinfo *fi = *i;
                if (fi)
                    right->getFunctionsDB()->DeleteFolder(fi->first);
            }
            right->getChosenFolders().clear();
            ////////right->ChangeFolderDB(right->getCurrentFolder());
        }
    }
    left->setSelectionMode(QAbstractItemView::NoSelection);
    right->setSelectionMode(QAbstractItemView::NoSelection);
    left->update();
    right->update();
    if (left->getIfDB())
        left->refreshDB();
    if (right->getIfDB())
        right->refreshDB();
}

void Workspace::copy()
{
    if (isLeftCurrent)
    {
        if (!left->getIfDB())
        {
            if(right->getIfDB())
            {
                QModelIndexList il = left->getList();
                for(auto i = il.begin(); i != il.end(); i++)
                {
                    right->getFunctionsDB()->CopyFileToDB(left->getFilesystem()->fileInfo(*i).absoluteFilePath(), right->getCurrentFolder());
                }
            }
            else
            {
                QModelIndexList il = left->getList();
                for(QModelIndexList::iterator i = il.begin(); i != il.end(); i++)
                {
                    left->getFilesystem()->copyIndex(*i, right->getPath());
                }
            }
        }
        else
        {
            if (right->getIfDB())
            {
                if (left->getFunctionsDB()->getDataBase()->databaseName() != right->getFunctionsDB()->getDataBase()->databaseName())
                {
                    for (auto i = left->getChosenItems().begin(); i != left->getChosenItems().end(); i++)
                    {
                        left->getFunctionsDB()->CopyItemBetweenDatabases(*i,
                                                                         left->getFunctionsDB()->getDataBase(),
                                                                         right->getFunctionsDB()->getDataBase(),
                                                                         right->getCurrentFolder());
                    }
                    for (auto i = left->getChosenFolders().begin(); i != left->getChosenFolders().end(); i++)
                    {
                        left->getFunctionsDB()->CopyFolder((*i)->first, left->getCurrentFolder(), right->getCurrentFolder());
                    }
                }
                else
                {
                    for (auto i = left->getChosenItems().begin(); i != left->getChosenItems().end(); i++)
                    {
                        left->getFunctionsDB()->CopyItemWithinDatabase(*i, left->getCurrentFolder(), right->getCurrentFolder());
                    }
                    for (auto i = left->getChosenFolders().begin(); i != left->getChosenFolders().end(); i++)
                    {
                        left->getFunctionsDB()->CopyFolder((*i)->first, left->getCurrentFolder(), right->getCurrentFolder());
                    }
                }
            }
            else
            {
                for (auto i = left->getChosenItems().begin(); i != left->getChosenItems().end(); i++)
                {
                    QString string = right->getPath();
                    string.append("/");
                    string.append((*i)->name.replace(':', '.').replace('/', '.').replace('\\', '.'));
                    string.append(".xml");
                    left->getFunctionsDB()->CopyItemToFile(*i, string);
                }
                for (auto i = left->getChosenFolders().begin(); i != left->getChosenFolders().end(); i++)
                {
                    left->getFunctionsDB()->CopyFolder((*i)->first, left->getCurrentFolder(), right->getCurrentFolder());//тут мы должны рекурсивно копировать папки из БД в Файловую систему, но такой функции в TIPDBSHELL нет
                }
            }
        }
    }
    else
    {
        if (!right->getIfDB())
        {

            if(left->getIfDB())
            {
                QModelIndexList il = right->getList();
                for(auto i = il.begin(); i != il.end(); i++)
                {
                    right->getFunctionsDB()->CopyFileToDB(filesystem->fileInfo(*i).absoluteFilePath(), left->getCurrentFolder());
                }
            }
            else
            {
                QModelIndexList il = right->getList();
                for(auto i = il.begin(); i != il.end(); i++)
                {
                    right->getFilesystem()->copyIndex(*i, left->getPath());
                }
            }
        }
        else
        {
            if (left->getIfDB())
            {
                for (auto i = right->getChosenItems().begin(); i != right->getChosenItems().end(); i++)
                {
                    right->getFunctionsDB()->CopyItemWithinDatabase(*i, right->getCurrentFolder(), left->getCurrentFolder());
                }
                for (auto i = right->getChosenFolders().begin(); i != right->getChosenFolders().end(); i++)
                {
                    right->getFunctionsDB()->CopyFolder((*i)->first, right->getCurrentFolder(), left->getCurrentFolder());
                }
            }
            else
            {
                for (auto i = right->getChosenItems().begin(); i != right->getChosenItems().end(); i++)
                {
                    QString string = left->getPath();
                    string.append("/");
                    string.append((*i)->name.replace(':', '.').replace('/', '.').replace('\\', '.'));
                    string.append(".xml");
                    right->getFunctionsDB()->CopyItemToFile(*i, string);
                }
                for (auto i = right->getChosenFolders().begin(); i != right->getChosenFolders().end(); i++)
                {
                    right->getFunctionsDB()->CopyFolder((*i)->first, right->getCurrentFolder(), left->getCurrentFolder());//тут мы должны копировать папки из БД в Файловую систему, но такой функции в TIPDBSHELL нет
                }
            }
        }
    }
    left->update();
    right->update();
    if (left->getIfDB())
        left->refreshDB();
    if (right->getIfDB())
        right->refreshDB();
}
void Workspace::move()
{
    if (isLeftCurrent)
    {
        if (!left->getIfDB())
        {

            if(right->getIfDB())
            {
                for(auto i = left->getList().begin(); i != left->getList().end(); i++)
                {
                    left->getFunctionsDB()->CopyFileToDB(left->getFilesystem()->fileInfo(*i).absoluteFilePath(), right->getCurrentFolder());
                    left->getFilesystem()->removeIndex(*i);
                }
                left->getList().clear();
            }
            else
            {
                for(auto i = left->getList().begin(); i != left->getList().end(); i++)
                {
                    left->getFilesystem()->copyIndex(*i, right->getPath());
                    left->getFilesystem()->removeIndex(*i);
                }
            }
        }
        else
        {
            if (right->getIfDB())
            {
                for (auto i = left->getChosenItems().begin(); i != left->getChosenItems().end(); i++)
                {
                    left->getFunctionsDB()->CopyItemWithinDatabase(*i, left->getCurrentFolder(), right->getCurrentFolder());
                    left->getFunctionsDB()->DeleteItem(*i, left->getCurrentFolder());
                }
                for (auto i = left->getChosenFolders().begin(); i != left->getChosenFolders().end(); i++)
                {
                    left->getFunctionsDB()->CopyFolder((*i)->first, left->getCurrentFolder(), right->getCurrentFolder());
                    left->getFunctionsDB()->DeleteFolder((*i)->first);
                }
            }
            else
            {
                for (auto i = left->getChosenItems().begin(); i != left->getChosenItems().end(); i++)
                {
                    QString string = right->getPath();
                    string.append("/");
                    string.append((*i)->name.replace(':', '.').replace('/', '.').replace('\\', '.'));
                    string.append(".xml");
                    left->getFunctionsDB()->CopyItemToFile(*i, string);
                    left->getFunctionsDB()->DeleteItem(*i, left->getCurrentFolder());
                }
                for (auto i = left->getChosenFolders().begin(); i != left->getChosenFolders().end(); i++)
                {
                    left->getFunctionsDB()->CopyFolder((*i)->first, left->getCurrentFolder(), right->getCurrentFolder());//тут мы должны копировать папки из БД в Файловую систему, но такой функции в TIPDBSHELL нет
                    left->getFunctionsDB()->DeleteFolder((*i)->first);
                }
            }
        }
    }
    else
    {
        if (!right->getIfDB())
        {

            if(left->getIfDB())
            {
                for(auto i = right->getList().begin(); i != right->getList().end(); i++)
                {
                    right->getFunctionsDB()->CopyFileToDB(right->getFilesystem()->fileInfo(*i).absoluteFilePath(), left->getCurrentFolder());
                    right->getFilesystem()->removeIndex(*i);
                }
            }
            else
            {
                for(auto i = right->getList().begin(); i != right->getList().end(); i++)
                {
                    right->getFilesystem()->copyIndex(*i, left->getPath());
                    right->getFilesystem()->removeIndex(*i);
                }
            }
        }
        else
        {
            if (left->getIfDB())
            {
                for (auto i = right->getChosenItems().begin(); i != right->getChosenItems().end(); i++)
                {
                    right->getFunctionsDB()->CopyItemWithinDatabase(*i, right->getCurrentFolder(), left->getCurrentFolder());
                    right->getFunctionsDB()->DeleteItem(*i, right->getCurrentFolder());
                }
                for (auto i = right->getChosenFolders().begin(); i != right->getChosenFolders().end(); i++)
                {
                    right->getFunctionsDB()->CopyFolder((*i)->first, right->getCurrentFolder(), left->getCurrentFolder());
                    right->getFunctionsDB()->DeleteFolder((*i)->first);
                }
            }
            else
            {
                for (auto i = right->getChosenItems().begin(); i != right->getChosenItems().end(); i++)
                {
                    QString string = left->getPath();
                    string.append("/");
                    string.append((*i)->name.replace(':', '.').replace('/', '.').replace('\\', '.'));
                    string.append(".xml");
                    right->getFunctionsDB()->CopyItemToFile(*i, string);
                    right->getFunctionsDB()->DeleteItem(*i, right->getCurrentFolder());
                }
                for (auto i = right->getChosenFolders().begin(); i != right->getChosenFolders().end(); i++)
                {
                    right->getFunctionsDB()->CopyFolder((*i)->first, right->getCurrentFolder(), left->getCurrentFolder());//тут мы должны копировать папки из БД в Файловую систему, но такой функции в TIPDBSHELL нет
                    right->getFunctionsDB()->DeleteFolder((*i)->first);
                }
            }
        }
    }
    left->setSelectionMode(QAbstractItemView::NoSelection);
    right->setSelectionMode(QAbstractItemView::NoSelection);
    left->changeDirectory(left->rootIndex());
    right->changeDirectory(right->rootIndex());
    left->update();
    right->update();
    if (left->getIfDB())
        left->refreshDB();
    if (right->getIfDB())
        right->refreshDB();
}

void Workspace::updateInfo(bool isLeft, bool isPlus, QModelIndex index)
{
    if (filesystem->fileName(index) == "..")
        return;
    if (isLeft)
    {
        isLeftCurrent = true;
        if (!left->getIfDB())
        {
        if (filesystem->isDir(index))
        {
            left->changeSize(isPlus, 0);
            left->changeCountChosenFolders(isPlus);
        }
        else
        {
            left->changeSize(isPlus, filesystem->size(index) / 1024);
            left->changeCountChosenFiles(isPlus);
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
        if (!right->getIfDB())
        {
        if (filesystem->isDir(index))
        {
            right->changeSize(isPlus, 0);
            right->changeCountChosenFolders(isPlus);
        }
        else
        {
            right->changeSize(isPlus, filesystem->size(index) / 1024);
            right->changeCountChosenFiles(isPlus);
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
    if (isLeft && left->getIfDB())
    {
        return;
    }
    else if (!isLeft && right->getIfDB())
    {
        return;
    }
    else
    {
    updateFolder(isLeft, filesystem->fileInfo(index).absoluteFilePath());
}
}
void Workspace::updateFolder(bool isLeft, QString path)
{
    QDir file;
    file.setPath(path);
    QStringList files = file.entryList(QDir::Files);
    int countFolders = 0;
    int countFiles = 0;
    QFileInfo information(path);
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
        left->changeCurrentFolderInfo(path, size, countFiles, countFolders);
    }
    else
    {
        right->changeCurrentFolderInfo(path, size, countFiles, countFolders);
    }
}
void Workspace::changeSelectionMode()
{
    if (isLeftCurrent)
    {
        left->selectionModel()->clear();
        left->changeSelectionMode();
    }
    else
    {
        right->selectionModel()->clear();
        right->changeSelectionMode();
    }
}

void Workspace::changeCurrentPanel()
{
    if (isLeftCurrent)
    {
        isLeftCurrent = false;
        right->setFocus();
        if (!right->currentIndex().isValid())
             right->setCurrentIndex(right->model()->index(0, 0, right->rootIndex()));
    }
    else
    {
        isLeftCurrent = true;
        left->setFocus();
        if (!left->currentIndex().isValid())
             left->setCurrentIndex(left->model()->index(0, 0, left->rootIndex()));
    }
}

void Workspace::createDir()
{
    if (isLeftCurrent && !left->getIfDB())
    {
        QString newCatalog = QInputDialog::getText(0, "New catalog", "Edit new catalog name: ", QLineEdit::Normal, "");
        filesystem->mkdir(left->rootIndex(), newCatalog);
    }
    else if (!isLeftCurrent && !right->getIfDB())
    {
        QString newCatalog = QInputDialog::getText(0, "New catalog", "Edit new catalog name: ", QLineEdit::Normal, "");
        filesystem->mkdir(right->rootIndex(), newCatalog);
    }
    else if (isLeftCurrent && left->getIfDB())
    {
        QString newCatalog = QInputDialog::getText(0, "New catalog", "Edit new catalog name: ", QLineEdit::Normal, "");
        int new_id = left->getFunctionsDB()->NewFolder(newCatalog);
        QStandardItem *id = new QStandardItem(QString::number(new_id));
        QStandardItem *name = new QStandardItem(newCatalog);
        name->setIcon(QIcon("resources/folder.png"));
        left->getDB()->setItem(left->getFolders()->size() + left->getItems()->size(), 2, id);
        left->getDB()->setItem(left->getFolders()->size() + left->getItems()->size(), 0, name);
        folderinfo *fold = new folderinfo;
        fold->first = new_id;
        fold->second = newCatalog;
        left->getFolders()->push_back(fold);
    }
    else if (!isLeftCurrent && right->getIfDB())
    {
        QString newCatalog = QInputDialog::getText(0, "New catalog", "Edit new catalog name: ", QLineEdit::Normal, "");
        int new_id = right->getFunctionsDB()->NewFolder(newCatalog);
        QStandardItem *id = new QStandardItem(QString::number(new_id));
        QStandardItem *name = new QStandardItem(newCatalog);
        name->setIcon(QIcon("resources/folder.png"));
        right->getDB()->setItem(right->getFolders()->size() + right->getItems()->size(), 2, id);
        right->getDB()->setItem(right->getFolders()->size() + right->getItems()->size(), 0, name);
        folderinfo *fold = new folderinfo;
        fold->first = new_id;
        fold->second = newCatalog;
        right->getFolders()->push_back(fold);
    }

    left->update();
    right->update();
    if (left->getIfDB())
        left->refreshDB();
    if (right->getIfDB())
        right->refreshDB();
}

void Workspace::changeDir()
{
    if(isLeftCurrent)
    {
        left->changeDirectory(left->currentIndex());
        left->setFocus();
        left->setCurrentIndex(left->model()->index(0, 0, left->rootIndex()));
    }
    else
    {
        right->changeDirectory(right->currentIndex());
        right->setFocus();
    }
}

void Workspace::updateInfoDB(bool isLeft, bool isPlus)
{
    if (isLeft)
    {
        isLeftCurrent = true;
        for (auto i = left->getChosenItems().begin(); i != left->getChosenItems().end(); i++)
        {
            left->changeSize(isPlus, (*i)->sizeInBytes / 1024);
            left->changeCountChosenFiles(isPlus);
        }
        for (auto i = left->getChosenFolders().begin(); i != left->getChosenFolders().end(); i++)
        {
            left->changeSize(isPlus, 0);
            left->changeCountChosenFolders(isPlus);
        }
    }
    else
    {
        isLeftCurrent = false;
        for (auto i = right->getChosenItems().begin(); i != right->getChosenItems().end(); i++)
        {
            right->changeSize(isPlus, (*i)->sizeInBytes / 1024);
            right->changeCountChosenFiles(isPlus);
        }
        for (auto i = right->getChosenFolders().begin(); i != right->getChosenFolders().end(); i++)
        {
            right->changeSize(isPlus, 0);
            right->changeCountChosenFolders(isPlus);
        }
    }
}
