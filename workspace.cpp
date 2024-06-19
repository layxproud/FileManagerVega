#include "workspace.h"
#include <QFileDialog>
#include <QInputDialog>

Workspace::Workspace(Panel *left, Panel *right, FileSystem *filesystem, QObject *parent)
    : QObject(parent)
    , leftPanel(left)
    , rightPanel(right)
    , fileSystem(filesystem)
    , isLeftCurrent(true)
    , serviceHandler()
    , classifyWindow(nullptr)
    , indexWindow(nullptr)
    , clusterizeWindow(nullptr)
    , matchLevelWindow(nullptr)
    , matchWindow(nullptr)
{
    leftPanel->setFocus();
    leftPanel->setCurrentIndex(this->leftPanel->currentIndex());

    connect(leftPanel, &Panel::updateInfo, this, &Workspace::updateInfo);
    connect(rightPanel, &Panel::updateInfo, this, &Workspace::updateInfo);
    connect(leftPanel, &Panel::changeFolder, this, &Workspace::updateFolder);
    connect(rightPanel, &Panel::changeFolder, this, &Workspace::updateFolder);
}

Workspace::~Workspace() {}

void Workspace::setServiceHandler(ServiceHandler *sh)
{
    this->serviceHandler = sh;
}

bool Workspace::getIsLeftCurrent()
{
    return isLeftCurrent;
}

void Workspace::updatePanels()
{
    if (leftPanel->getIsDB()) {
        leftPanel->refreshDB();
    } else {
        leftPanel->refreshFS();
    }

    if (rightPanel->getIsDB()) {
        rightPanel->refreshDB();
    } else {
        rightPanel->refreshFS();
    }

    updateFolder(true, leftPanel->getPath(), leftPanel->getIsDB());
    updateFolder(false, rightPanel->getPath(), rightPanel->getIsDB());
}

void Workspace::choose()
{
    if (isLeftCurrent) {
        leftPanel->chooseButton();
    } else {
        rightPanel->chooseButton();
    }
}

void Workspace::remove()
{
    if (isLeftCurrent) {
        if (!leftPanel->getIsDB()) {
            removeFilesystemEntries(leftPanel);
        } else {
            removeDatabaseEntries(leftPanel);
        }
    } else {
        if (!rightPanel->getIsDB()) {
            removeFilesystemEntries(rightPanel);
        } else {
            removeDatabaseEntries(rightPanel);
        }
    }

    updatePanels();
}

void Workspace::removeFilesystemEntries(Panel *panel)
{
    QModelIndexList &selectedIndexes = panel->getList();
    if (selectedIndexes.isEmpty()) {
        return;
    }

    for (const auto &index : selectedIndexes) {
        fileSystem->removeIndex(index);
    }
    selectedIndexes.clear();
}

void Workspace::removeDatabaseEntries(Panel *panel)
{
    for (const auto &item : panel->getChosenItems()) {
        serviceHandler
            ->deleteDbEntry(item->id, panel->getFunctionsDB()->getDataBase()->databaseName());
    }
    panel->getChosenItems().clear();

    for (auto i = panel->getChosenFolders().begin(); i != panel->getChosenFolders().end(); i++) {
        folderinfo *fi = *i;
        if (fi) {
            panel->getFunctionsDB()->DeleteFolder(fi->first);
        }
    }
    panel->getChosenFolders().clear();
}

void Workspace::copy()
{
    if (isLeftCurrent) {
        if (!leftPanel->getIsDB()) {
            if (rightPanel->getIsDB()) {
                copyFilesystemToDatabase(leftPanel, rightPanel);
            } else {
                copyFilesystemToFilesystem(leftPanel, rightPanel);
            }
        } else {
            if (rightPanel->getIsDB()) {
                copyDatabaseToDatabase(leftPanel, rightPanel);
            } else {
                copyDatabaseToFilesystem(leftPanel, rightPanel);
            }
        }
    } else {
        if (!rightPanel->getIsDB()) {
            if (leftPanel->getIsDB()) {
                copyFilesystemToDatabase(rightPanel, leftPanel);
            } else {
                copyFilesystemToFilesystem(rightPanel, leftPanel);
            }
        } else {
            if (leftPanel->getIsDB()) {
                copyDatabaseToDatabase(rightPanel, leftPanel);
            } else {
                copyDatabaseToFilesystem(rightPanel, leftPanel);
            }
        }
    }
    updatePanels();
}

void Workspace::copyFilesystemToDatabase(Panel *sourcePanel, Panel *destinationPanel)
{
    QModelIndexList &allIndexes = sourcePanel->getList();
    QModelIndexList xmlFiles;
    QStringList filesToIndex;
    for (const auto &index : allIndexes) {
        QString filePath = sourcePanel->getFilesystem()->filePath(index);
        QString extension = QFileInfo(filePath).suffix().toLower();
        if (extension == "txt" || extension == "pdf" || extension == "html" || extension == "docx"
            || extension == "doc") {
            filesToIndex.append(filePath);
        } else if (extension == "xml") {
            xmlFiles.append(index);
        } else {
            continue;
        }
    }

    if (!filesToIndex.empty()) {
        if (indexWindow) {
            indexWindow->setFiles(filesToIndex);
            indexWindow->raise();
            indexWindow->activateWindow();
        } else {
            indexWindow = new IndexWindow();
            widgetsList.append(indexWindow);
            connect(indexWindow, &QObject::destroyed, this, &Workspace::handleWidgetDestroyed);
            connect(
                indexWindow,
                &IndexWindow::indexFiles,
                serviceHandler,
                &ServiceHandler::indexFilesSignal);

            connect(indexWindow, &IndexWindow::addFilesSignal, [this]() {
                Panel *_panel = isLeftCurrent ? leftPanel : rightPanel;
                if (_panel->getIsDB()) {
                    qWarning() << "Выберите файлы в панели файловой системы";
                    return;
                }
                QModelIndexList &_allIndexes = _panel->getList();
                QStringList _filesToIndex;
                for (const auto &index : _allIndexes) {
                    QString filePath = _panel->getFilesystem()->filePath(index);
                    QString extension = QFileInfo(filePath).suffix().toLower();
                    if (extension == "txt" || extension == "pdf" || extension == "html"
                        || extension == "docx" || extension == "doc") {
                        _filesToIndex.append(filePath);
                    }
                }
                indexWindow->setFiles(_filesToIndex);
            });

            indexWindow->setFiles(filesToIndex);
            indexWindow->setDbName(
                destinationPanel->getFunctionsDB()->getDataBase()->databaseName());
        }
        indexWindow->show();
    }

    for (const auto &file : xmlFiles) {
        destinationPanel->getFunctionsDB()->CopyFileToDB(
            sourcePanel->getFilesystem()->fileInfo(file).absoluteFilePath(),
            destinationPanel->getCurrentFolder());
    }
}

void Workspace::copyFilesystemToFilesystem(Panel *sourcePanel, Panel *destinationPanel)
{
    QModelIndexList &il = sourcePanel->getList();
    for (QModelIndexList::iterator i = il.begin(); i != il.end(); i++) {
        sourcePanel->getFilesystem()->copyIndex(*i, destinationPanel->getPath());
    }
}

void Workspace::copyDatabaseToDatabase(Panel *sourcePanel, Panel *destinationPanel)
{
    if (sourcePanel->getFunctionsDB()->getDataBase()->databaseName()
        != destinationPanel->getFunctionsDB()->getDataBase()->databaseName()) {
        for (auto i = sourcePanel->getChosenItems().begin();
             i != sourcePanel->getChosenItems().end();
             i++) {
            sourcePanel->getFunctionsDB()->CopyItemBetweenDatabases(
                *i,
                sourcePanel->getFunctionsDB()->getDataBase(),
                destinationPanel->getFunctionsDB()->getDataBase(),
                destinationPanel->getCurrentFolder());
        }
        for (auto i = sourcePanel->getChosenFolders().begin();
             i != sourcePanel->getChosenFolders().end();
             i++) {
            sourcePanel->getFunctionsDB()->CopyFolder(
                (*i)->first, sourcePanel->getCurrentFolder(), destinationPanel->getCurrentFolder());
        }
    } else {
        for (auto i = sourcePanel->getChosenItems().begin();
             i != sourcePanel->getChosenItems().end();
             i++) {
            sourcePanel->getFunctionsDB()->CopyItemWithinDatabase(
                *i, sourcePanel->getCurrentFolder(), destinationPanel->getCurrentFolder());
        }
        for (auto i = sourcePanel->getChosenFolders().begin();
             i != sourcePanel->getChosenFolders().end();
             i++) {
            sourcePanel->getFunctionsDB()->CopyFolder(
                (*i)->first, sourcePanel->getCurrentFolder(), destinationPanel->getCurrentFolder());
        }
    }
}

void Workspace::copyDatabaseToFilesystem(Panel *sourcePanel, Panel *destinationPanel)
{
    for (auto i = sourcePanel->getChosenItems().begin(); i != sourcePanel->getChosenItems().end();
         i++) {
        QString string = destinationPanel->getPath();
        string.append("/");
        string.append((*i)->name.replace(':', '.').replace('/', '.').replace('\\', '.'));
        string.append(".xml");
        sourcePanel->getFunctionsDB()->CopyItemToFile(*i, string);
    }

    for (auto i = sourcePanel->getChosenFolders().begin();
         i != sourcePanel->getChosenFolders().end();
         i++) {
        sourcePanel->getFunctionsDB()->CopyFolder(
            (*i)->first, sourcePanel->getCurrentFolder(), destinationPanel->getCurrentFolder());
    }
}

void Workspace::move()
{
    if (isLeftCurrent) {
        if (!leftPanel->getIsDB()) {
            if (rightPanel->getIsDB()) {
                moveFilesystemToDatabase(leftPanel, rightPanel);
            } else {
                moveFilesystemToFilesystem(leftPanel, rightPanel);
            }
        } else {
            if (rightPanel->getIsDB()) {
                moveDatabaseToDatabase(leftPanel, rightPanel);
            } else {
                moveDatabaseToFilesystem(leftPanel, rightPanel);
            }
        }
    } else {
        if (!rightPanel->getIsDB()) {
            if (leftPanel->getIsDB()) {
                moveFilesystemToDatabase(rightPanel, leftPanel);
            } else {
                moveFilesystemToFilesystem(rightPanel, leftPanel);
            }
        } else {
            if (leftPanel->getIsDB()) {
                moveDatabaseToDatabase(rightPanel, leftPanel);
            } else {
                moveDatabaseToFilesystem(rightPanel, leftPanel);
            }
        }
    }

    leftPanel->changeDirectory(leftPanel->rootIndex());
    rightPanel->changeDirectory(rightPanel->rootIndex());

    updatePanels();
}

void Workspace::moveFilesystemToDatabase(Panel *sourcePanel, Panel *destinationPanel)
{
    QModelIndexList &il = sourcePanel->getList();
    for (auto i = il.begin(); i != il.end(); i++) {
        destinationPanel->getFunctionsDB()->CopyFileToDB(
            sourcePanel->getFilesystem()->fileInfo(*i).absoluteFilePath(),
            destinationPanel->getCurrentFolder());
        sourcePanel->getFilesystem()->removeIndex(*i);
    }
    sourcePanel->getList().clear();
}

void Workspace::moveFilesystemToFilesystem(Panel *sourcePanel, Panel *destinationPanel)
{
    QModelIndexList &il = sourcePanel->getList();
    for (auto i = il.begin(); i != il.end(); i++) {
        sourcePanel->getFilesystem()->copyIndex(*i, destinationPanel->getPath());
        sourcePanel->getFilesystem()->removeIndex(*i);
    }
}

void Workspace::moveDatabaseToDatabase(Panel *sourcePanel, Panel *destinationPanel)
{
    copyDatabaseToDatabase(sourcePanel, destinationPanel);
    for (auto i = sourcePanel->getChosenItems().begin(); i != sourcePanel->getChosenItems().end();
         i++) {
        sourcePanel->getFunctionsDB()->DeleteItem(*i, sourcePanel->getCurrentFolder());
    }
    for (auto i = sourcePanel->getChosenFolders().begin();
         i != sourcePanel->getChosenFolders().end();
         i++) {
        sourcePanel->getFunctionsDB()->DeleteFolder((*i)->first);
    }
}

void Workspace::moveDatabaseToFilesystem(Panel *sourcePanel, Panel *destinationPanel)
{
    for (auto i = sourcePanel->getChosenItems().begin(); i != sourcePanel->getChosenItems().end();
         i++) {
        QString string = destinationPanel->getPath();
        string.append("/");
        string.append((*i)->name.replace(':', '.').replace('/', '.').replace('\\', '.'));
        string.append(".xml");
        sourcePanel->getFunctionsDB()->CopyItemToFile(*i, string);
        sourcePanel->getFunctionsDB()->DeleteItem(*i, sourcePanel->getCurrentFolder());
    }

    for (auto i = sourcePanel->getChosenFolders().begin();
         i != sourcePanel->getChosenFolders().end();
         i++) {
        sourcePanel->getFunctionsDB()->CopyFolder(
            (*i)->first, sourcePanel->getCurrentFolder(), destinationPanel->getCurrentFolder());
        sourcePanel->getFunctionsDB()->DeleteFolder((*i)->first);
    }
}

void Workspace::createDir()
{
    if (isLeftCurrent && !leftPanel->getIsDB()) {
        createDirFileSystem(leftPanel);
    } else if (!isLeftCurrent && !rightPanel->getIsDB()) {
        createDirFileSystem(rightPanel);
    } else if (isLeftCurrent && leftPanel->getIsDB()) {
        createDirDatabase(leftPanel);
    } else if (!isLeftCurrent && rightPanel->getIsDB()) {
        createDirDatabase(rightPanel);
    }

    updatePanels();
}

void Workspace::createDirFileSystem(Panel *panel)
{
    QString newCatalog = QInputDialog::getText(
        0, tr("Новая папка"), tr("Введите название: "), QLineEdit::Normal, "");

    if (newCatalog.isEmpty()) {
        return;
    }

    QSortFilterProxyModel *proxyModel = qobject_cast<QSortFilterProxyModel *>(panel->model());
    if (!proxyModel) {
        qWarning() << "Panel model is not a QSortFilterProxyModel";
        return;
    }

    QModelIndex proxyIndex = panel->rootIndex();
    QModelIndex sourceIndex = proxyModel->mapToSource(proxyIndex);

    QFileSystemModel *fileSystemModel = qobject_cast<QFileSystemModel *>(proxyModel->sourceModel());
    if (!fileSystemModel) {
        qWarning() << "Source model is not a QFileSystemModel";
        return;
    }

    QModelIndex newDirIndex = fileSystemModel->mkdir(sourceIndex, newCatalog);

    if (!newDirIndex.isValid()) {
        qCritical() << "Не удалось создать директорию!";
        return;
    }

    updateFolder(isLeftCurrent, panel->getPath(), false);
}

void Workspace::createDirDatabase(Panel *panel)
{
    QString newCatalog = QInputDialog::getText(
        0, tr("Новая коллекция"), tr("Введите название: "), QLineEdit::Normal, "");

    if (newCatalog.isEmpty()) {
        return;
    }

    int new_id = panel->getFunctionsDB()->NewFolder(newCatalog);
    QStandardItem *id = new QStandardItem(QString::number(new_id));
    QStandardItem *name = new QStandardItem(newCatalog);
    name->setIcon(QIcon(":/icons/resources/folder.png"));
    panel->getDB()->setItem(panel->getFolders()->size() + panel->getItems()->size(), 1, id);
    panel->getDB()->setItem(panel->getFolders()->size() + panel->getItems()->size(), 0, name);
    folderinfo *fold = new folderinfo;
    fold->first = new_id;
    fold->second = newCatalog;
    panel->getFolders()->push_back(fold);
}

bool Workspace::checkServiceHandler()
{
    if (!serviceHandler) {
        qCritical() << "Проблема с модулем веб-сервисов";
        return false;
    }
    return true;
}

bool Workspace::checkPanels(bool bothPanelsNeedDB)
{
    if (bothPanelsNeedDB) {
        if (!leftPanel->getIsDB() || !rightPanel->getIsDB()) {
            qWarning() << "В обоих панелях должна быть открыта база данных";
            return false;
        }
    } else {
        if (!leftPanel->getIsDB() && !rightPanel->getIsDB()) {
            qWarning() << "Хотя бы одна панель должна быть базой данных";
            return false;
        }
    }

    if (leftPanel->getChosenItems().empty() && rightPanel->getChosenItems().empty()) {
        qWarning() << "Не выбрано ни одного элемента";
        return false;
    }

    return true;
}

std::pair<QString, std::list<TIPInfo *>> Workspace::getChosenItemsAndDbName()
{
    QString dbName;
    std::list<TIPInfo *> chosenItems;

    if (isLeftCurrent) {
        dbName = leftPanel->getFunctionsDB()->getDataBase()->databaseName();
        chosenItems = leftPanel->getChosenItems();
    } else {
        dbName = rightPanel->getFunctionsDB()->getDataBase()->databaseName();
        chosenItems = rightPanel->getChosenItems();
    }

    return std::make_pair(dbName, chosenItems);
}

void Workspace::changeDir()
{
    if (isLeftCurrent) {
        leftPanel->changeDirectory(leftPanel->currentIndex());
        leftPanel->setFocus();
        leftPanel->setCurrentIndex(leftPanel->model()->index(0, 0, leftPanel->rootIndex()));
    } else {
        rightPanel->changeDirectory(rightPanel->currentIndex());
        rightPanel->setFocus();
    }
}

void Workspace::updateInfo(bool isLeft, bool isPlus, QModelIndex index)
{
    if (fileSystem->fileName(index) == "..") {
        return;
    }
    if (isLeft) {
        isLeftCurrent = true;
        if (!leftPanel->getIsDB()) {
            if (fileSystem->isDir(index)) {
                leftPanel->changeSize(isPlus, 0);
                leftPanel->changeCountChosenFolders(isPlus);
            } else {
                leftPanel->changeSize(isPlus, fileSystem->size(index) / 1024);
                leftPanel->changeCountChosenFiles(isPlus);
            }
        } else {
            // Обрабатывается в Panel
            return;
        }
    } else {
        isLeftCurrent = false;
        if (!rightPanel->getIsDB()) {
            if (fileSystem->isDir(index)) {
                rightPanel->changeSize(isPlus, 0);
                rightPanel->changeCountChosenFolders(isPlus);
            } else {
                rightPanel->changeSize(isPlus, fileSystem->size(index) / 1024);
                rightPanel->changeCountChosenFiles(isPlus);
            }
        } else {
            // Обрабатывается в Panel
            return;
        }
    }
}

void Workspace::updateFolder(bool isLeft, QString path, bool isDB)
{
    long long int size = 0;

    if (!isDB) {
        QDir currDir;
        currDir.setPath(path);
        QStringList files = currDir.entryList(QDir::Files);
        int countFolders = 0;
        int countFiles = 0;

        foreach (QFileInfo currEntry, currDir.entryInfoList()) {
            QString currEntryName(currEntry.fileName());
            if (currEntryName == "." || currEntryName == ".." || currEntryName.isEmpty()) {
                continue;
            }
            if (currEntry.isDir()) {
                countFolders++;
            } else {
                countFiles++;
                size += currEntry.size() / 1024;
            }
        }

        if (isLeft) {
            leftPanel->changeCurrentFolderInfo(path, size, countFiles, countFolders);
        } else {
            rightPanel->changeCurrentFolderInfo(path, size, countFiles, countFolders);
        }
    } else {
        std::vector<TIPInfo *> items;
        std::vector<folderinfo *> folders;

        if (isLeft) {
            leftPanel->getFunctionsDB()
                ->GetFolderContents(leftPanel->getCurrentFolder(), items, folders);
            for (const auto &item : items) {
                size += item->sizeInTerms;
            }
            leftPanel->changeCurrentFolderInfo(path, size, items.size(), folders.size());
        } else {
            rightPanel->getFunctionsDB()
                ->GetFolderContents(rightPanel->getCurrentFolder(), items, folders);
            for (const auto &item : items) {
                size += item->sizeInTerms;
            }
            rightPanel->changeCurrentFolderInfo(path, size, items.size(), folders.size());
        }
    }
}

void Workspace::changeSelectionMode()
{
    if (isLeftCurrent) {
        leftPanel->selectionModel()->clear();
        leftPanel->clearLists();
        leftPanel->changeSelectionMode();
    } else {
        rightPanel->selectionModel()->clear();
        rightPanel->clearLists();
        rightPanel->changeSelectionMode();
    }
}

void Workspace::changeCurrentPanel()
{
    if (isLeftCurrent) {
        isLeftCurrent = false;
        rightPanel->setFocus();
        if (!rightPanel->currentIndex().isValid()) {
            rightPanel->setCurrentIndex(rightPanel->model()->index(0, 0, rightPanel->rootIndex()));
        }
    } else {
        isLeftCurrent = true;
        leftPanel->setFocus();
        if (!leftPanel->currentIndex().isValid()) {
            leftPanel->setCurrentIndex(leftPanel->model()->index(0, 0, leftPanel->rootIndex()));
        }
    }
}

void Workspace::comparePortraits()
{
    if (!checkServiceHandler() || !checkPanels(true))
        return;

    auto result = getChosenItemsAndDbName();
    std::list<TIPInfo *> chosenItems = result.second;

    if (chosenItems.empty())
        return;

    auto leftPortrait = *(leftPanel->getChosenItems().rbegin());
    auto rightPortrait = *(rightPanel->getChosenItems().rbegin());

    auto fullLeftPortrait = leftPanel->getFunctionsDB()->loadFromDB(leftPortrait->id);
    auto fullRightPortrait = rightPanel->getFunctionsDB()->loadFromDB(rightPortrait->id);

    ipCompare = new IPCompare(
        serviceHandler->comparePortraits(fullLeftPortrait->terms, fullRightPortrait->terms));
    connect(ipCompare, &QObject::destroyed, this, &Workspace::handleWidgetDestroyed);
    ipCompare->show();
    widgetsList.append(ipCompare);
}

void Workspace::getXmlFile()
{
    if (!checkServiceHandler() || !checkPanels())
        return;

    QString filePath = QFileDialog::getSaveFileName(
        nullptr, tr("Сохранение XML файла"), "", "XML files(*.xml);;All Files(*)");

    if (filePath.isEmpty()) {
        qWarning() << "Файл не выбран";
        return;
    }

    auto result = getChosenItemsAndDbName();
    QString dbName = result.first;
    std::list<TIPInfo *> chosenItems = result.second;
    if (chosenItems.empty())
        return;
    long id = (*(chosenItems.rbegin()))->id;
    serviceHandler->getXmlFile(filePath, id, dbName);
}

void Workspace::getSummary()
{
    if (!checkServiceHandler() || !checkPanels())
        return;

    QString filePath = QFileDialog::getSaveFileName(
        nullptr, tr("Сохранение реферата"), "", "TXT files(*.txt);;All Files(*)");

    if (filePath.isEmpty()) {
        qWarning() << "Файл не выбран";
        return;
    }

    auto result = getChosenItemsAndDbName();
    QString dbName = result.first;
    std::list<TIPInfo *> chosenItems = result.second;
    if (chosenItems.empty())
        return;
    long id = (*(chosenItems.rbegin()))->id;
    serviceHandler->getSummary(filePath, id, dbName);
}

void Workspace::classifyPortraits()
{
    if (!checkServiceHandler() || !checkPanels())
        return;

    auto result = getChosenItemsAndDbName();
    QString dbName = result.first;
    std::list<TIPInfo *> chosenItems = result.second;
    if (chosenItems.empty())
        return;

    QMap<long, QString> portraitsMap;
    for (auto portrait : chosenItems) {
        portraitsMap.insert(portrait->id, portrait->name);
    }

    if (classifyWindow) {
        classifyWindow->setClasses(portraitsMap);
        classifyWindow->raise();
        classifyWindow->activateWindow();
    } else {
        classifyWindow = new ClassifyWindow();
        widgetsList.append(classifyWindow);
        connect(classifyWindow, &QObject::destroyed, this, &Workspace::handleWidgetDestroyed);

        connect(
            classifyWindow,
            &ClassifyWindow::classifyPortraits,
            serviceHandler,
            &ServiceHandler::classifyPortraitsSignal);

        connect(classifyWindow, &ClassifyWindow::getClassesSignal, [this]() {
            auto result = getChosenItemsAndDbName();
            std::list<TIPInfo *> chosenItems = result.second;
            QMap<long, QString> classesMap;
            for (auto portrait : chosenItems) {
                classesMap.insert(portrait->id, portrait->name);
            }
            classifyWindow->setClasses(classesMap);
        });

        connect(classifyWindow, &ClassifyWindow::getPortraitsSignal, [this]() {
            auto result = getChosenItemsAndDbName();
            std::list<TIPInfo *> chosenItems = result.second;
            QMap<long, QString> _portraitsMap;
            for (auto portrait : chosenItems) {
                _portraitsMap.insert(portrait->id, portrait->name);
            }
            classifyWindow->setPortraits(_portraitsMap);
        });

        connect(
            serviceHandler,
            &ServiceHandler::classificationCompleteSignal,
            classifyWindow,
            &ClassifyWindow::onClassificationComplete);

        classifyWindow->setClasses(portraitsMap);
        classifyWindow->setDbName(dbName);
    }

    classifyWindow->show();
}

void Workspace::clusterizePortraits()
{
    if (!checkServiceHandler() || !checkPanels())
        return;

    auto result = getChosenItemsAndDbName();
    QString dbName = result.first;
    std::list<TIPInfo *> chosenItems = result.second;
    if (chosenItems.empty())
        return;

    QMap<long, QString> portraitsMap;
    for (auto portrait : chosenItems) {
        portraitsMap.insert(portrait->id, portrait->name);
    }

    if (clusterizeWindow) {
        clusterizeWindow->setPortraits(portraitsMap);
        clusterizeWindow->raise();
        clusterizeWindow->activateWindow();
    } else {
        clusterizeWindow = new ClusterizeWindow();
        widgetsList.append(clusterizeWindow);
        connect(clusterizeWindow, &QObject::destroyed, this, &Workspace::handleWidgetDestroyed);

        connect(
            clusterizeWindow,
            &ClusterizeWindow::clusterizePortraits,
            serviceHandler,
            &ServiceHandler::clusterizePortraitsSignal);

        connect(clusterizeWindow, &ClusterizeWindow::addPortraitsSignal, [this]() {
            auto result = getChosenItemsAndDbName();
            std::list<TIPInfo *> chosenItems = result.second;
            QMap<long, QString> _portraitsMap;
            for (auto portrait : chosenItems) {
                _portraitsMap.insert(portrait->id, portrait->name);
            }
            clusterizeWindow->setPortraits(_portraitsMap);
        });

        connect(
            serviceHandler,
            &ServiceHandler::clusterizationCompleteSignal,
            clusterizeWindow,
            &ClusterizeWindow::onClusterizationComplete);

        clusterizeWindow->setPortraits(portraitsMap);
        clusterizeWindow->setDbName(dbName);
    }
    clusterizeWindow->show();
}

void Workspace::findMatch()
{
    if (!checkServiceHandler())
        return;

    if ((isLeftCurrent && !leftPanel->getIsDB()) || (!isLeftCurrent && !rightPanel->getIsDB())) {
        return;
    }

    auto result = getChosenItemsAndDbName();
    QString dbName = result.first;
    std::list<TIPInfo *> chosenItems = result.second;

    QMap<long, QString> portraitsMap;
    for (auto portrait : chosenItems) {
        portraitsMap.insert(portrait->id, portrait->name);
    }

    if (!matchWindow) {
        matchWindow = new MatchWindow();
        widgetsList.append(matchWindow);
        connect(matchWindow, &QObject::destroyed, this, &Workspace::handleWidgetDestroyed);

        connect(
            matchWindow,
            &MatchWindow::findMatchingPortraits,
            serviceHandler,
            &ServiceHandler::findMatchingPortraitsSignal);

        connect(matchWindow, &MatchWindow::addPortraitsSignal, [this]() {
            auto result = getChosenItemsAndDbName();
            std::list<TIPInfo *> chosenItems = result.second;
            QMap<long, QString> _portraitsMap;
            for (auto portrait : chosenItems) {
                _portraitsMap.insert(portrait->id, portrait->name);
            }
            matchWindow->setPortraits(_portraitsMap);
        });

        connect(
            serviceHandler,
            &ServiceHandler::findMatchingCompleteSignal,
            matchWindow,
            &MatchWindow::onFindMatchingComplete);

        matchWindow->setDbName(dbName);
        matchWindow->setPortraits(portraitsMap);
    }
    matchWindow->show();
    matchWindow->raise();
    matchWindow->activateWindow();
}

void Workspace::findMatchLevel()
{
    if (!checkServiceHandler() || !checkPanels())
        return;

    auto result = getChosenItemsAndDbName();
    QString dbName = result.first;
    std::list<TIPInfo *> chosenItems = result.second;

    if (chosenItems.empty())
        return;

    auto portrait = *(chosenItems.rbegin());

    if (!matchLevelWindow) {
        matchLevelWindow = new MatchLevelWindow();
        widgetsList.append(matchLevelWindow);
        connect(matchLevelWindow, &QObject::destroyed, this, &Workspace::handleWidgetDestroyed);

        connect(matchLevelWindow, &MatchLevelWindow::addPortraitsSignal, [this]() {
            auto result = getChosenItemsAndDbName();
            std::list<TIPInfo *> chosenItems = result.second;
            auto portrait = *(chosenItems.rbegin());
            matchLevelWindow->setMatchPortrait(portrait->name, portrait->id);
        });

        connect(
            matchLevelWindow,
            &MatchLevelWindow::findMatchLevelSignal,
            serviceHandler,
            &ServiceHandler::findMatchLevelSignal);

        connect(
            serviceHandler,
            &ServiceHandler::matchLevelCompleteSignal,
            matchLevelWindow,
            &MatchLevelWindow::onMatchLevelComplete);

        matchLevelWindow->setDbName(dbName);
        matchLevelWindow->setPortrait(portrait->name, portrait->id);
    }

    matchLevelWindow->show();
    matchLevelWindow->raise();
    matchLevelWindow->activateWindow();
}

void Workspace::killChildren()
{
    foreach (QWidget *widget, widgetsList) {
        delete widget;
    }
    widgetsList.clear();
}

void Workspace::handleWidgetDestroyed(QObject *object)
{
    QWidget *widget = qobject_cast<QWidget *>(object);
    if (widget == indexWindow) {
        indexWindow = nullptr;
    } else if (widget == classifyWindow) {
        classifyWindow = nullptr;
    } else if (widget == clusterizeWindow) {
        clusterizeWindow = nullptr;
    } else if (widget == matchLevelWindow) {
        matchLevelWindow = nullptr;
    } else if (widget == matchWindow) {
        matchWindow = nullptr;
    }
    if (widget && widgetsList.contains(widget)) {
        widgetsList.removeOne(widget);
    }
}
