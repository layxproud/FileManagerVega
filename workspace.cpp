#include "workspace.h"

Workspace::Workspace(Panel *left, Panel *right, FileSystem *filesystem, QObject *parent) :
    QObject(parent),
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

Workspace::~Workspace()
{
}


void Workspace::updatePanels()
{
    leftPanel->update();
    rightPanel->update();

    if (leftPanel->getIsDB())
        leftPanel->refreshDB();

    if (rightPanel->getIsDB())
        rightPanel->refreshDB();

    updateFolder(true, leftPanel->getPath());
    updateFolder(false, rightPanel->getPath());
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
        if (!leftPanel->getIsDB())
        {
            removeFilesystemEntries(leftPanel);
        }
        else
        {
            removeDatabaseEntries(leftPanel);
        }
    }
    else
    {
        if (!rightPanel->getIsDB())
        {
            removeFilesystemEntries(rightPanel);
        }
        else
        {
            removeDatabaseEntries(rightPanel);
        }
    }

    leftPanel->setSelectionMode(QAbstractItemView::NoSelection);
    rightPanel->setSelectionMode(QAbstractItemView::NoSelection);

    updatePanels();
}

void Workspace::removeFilesystemEntries(Panel* panel)
{
    QModelIndexList& selectedIndexes = panel->getList();

    foreach (const QModelIndex &index, selectedIndexes)
    {
        fileSystem->removeIndex(index);
    }

    selectedIndexes.clear();
}

void Workspace::removeDatabaseEntries(Panel* panel)
{
    for (auto i = panel->getChosenItems().begin(); i != panel->getChosenItems().end(); i++)
    {
        panel->getFunctionsDB()->DeleteItem(*i, panel->getCurrentFolder());
    }
    panel->getChosenItems().clear();

    for (auto i = panel->getChosenFolders().begin(); i != panel->getChosenFolders().end(); i++)
    {
        folderinfo *fi = *i;
        if (fi)
            panel->getFunctionsDB()->DeleteFolder(fi->first);
    }
    panel->getChosenFolders().clear();
}

void Workspace::copy()
{
    if (isLeftCurrent)
    {
        if (!leftPanel->getIsDB())
        {
            if (rightPanel->getIsDB())
            {
                copyFilesystemToDatabase(leftPanel, rightPanel);
            }
            else
            {
                copyFilesystemToFilesystem(leftPanel, rightPanel);
            }
        }
        else
        {
            if (rightPanel->getIsDB())
            {
                copyDatabaseToDatabase(leftPanel, rightPanel);
            }
            else
            {
                copyDatabaseToFilesystem(leftPanel, rightPanel);
            }
        }
    }
    else
    {
        if (!rightPanel->getIsDB())
        {
            if (leftPanel->getIsDB())
            {
                copyFilesystemToDatabase(rightPanel, leftPanel);
            }
            else
            {
                copyFilesystemToFilesystem(rightPanel, leftPanel);
            }
        }
        else
        {
            if (leftPanel->getIsDB())
            {
                copyDatabaseToDatabase(rightPanel, leftPanel);
            }
            else
            {
                copyDatabaseToFilesystem(rightPanel, leftPanel);
            }
        }
    }

    updatePanels();
}

void Workspace::copyFilesystemToDatabase(Panel* sourcePanel, Panel* destinationPanel)
{
    QModelIndexList& il = sourcePanel->getList();
    for (auto i = il.begin(); i != il.end(); i++)
    {
        destinationPanel->getFunctionsDB()->CopyFileToDB(sourcePanel->getFilesystem()->fileInfo(*i).absoluteFilePath(), destinationPanel->getCurrentFolder());
    }
}

void Workspace::copyFilesystemToFilesystem(Panel* sourcePanel, Panel* destinationPanel)
{
    QModelIndexList& il = sourcePanel->getList();
    for (QModelIndexList::iterator i = il.begin(); i != il.end(); i++)
    {
        sourcePanel->getFilesystem()->copyIndex(*i, destinationPanel->getPath());
    }
}

void Workspace::copyDatabaseToDatabase(Panel* sourcePanel, Panel* destinationPanel)
{
    if (sourcePanel->getFunctionsDB()->getDataBase()->databaseName() != destinationPanel->getFunctionsDB()->getDataBase()->databaseName())
    {
        for (auto i = sourcePanel->getChosenItems().begin(); i != sourcePanel->getChosenItems().end(); i++)
        {
            sourcePanel->getFunctionsDB()->CopyItemBetweenDatabases(*i,
                                                                    sourcePanel->getFunctionsDB()->getDataBase(),
                                                                    destinationPanel->getFunctionsDB()->getDataBase(),
                                                                    destinationPanel->getCurrentFolder());
        }
        for (auto i = sourcePanel->getChosenFolders().begin(); i != sourcePanel->getChosenFolders().end(); i++)
        {
            sourcePanel->getFunctionsDB()->CopyFolder((*i)->first,
                                                      sourcePanel->getCurrentFolder(),
                                                      destinationPanel->getCurrentFolder());
        }
    }
    else
    {
        for (auto i = sourcePanel->getChosenItems().begin(); i != sourcePanel->getChosenItems().end(); i++)
        {
            sourcePanel->getFunctionsDB()->CopyItemWithinDatabase(*i,
                                                                  sourcePanel->getCurrentFolder(),
                                                                  destinationPanel->getCurrentFolder());
        }
        for (auto i = sourcePanel->getChosenFolders().begin(); i != sourcePanel->getChosenFolders().end(); i++)
        {
            sourcePanel->getFunctionsDB()->CopyFolder((*i)->first,
                                                      sourcePanel->getCurrentFolder(),
                                                      destinationPanel->getCurrentFolder());
        }
    }
}

void Workspace::copyDatabaseToFilesystem(Panel* sourcePanel, Panel* destinationPanel)
{
    for (auto i = sourcePanel->getChosenItems().begin(); i != sourcePanel->getChosenItems().end(); i++)
    {
        QString string = destinationPanel->getPath();
        string.append("/");
        string.append((*i)->name.replace(':', '.').replace('/', '.').replace('\\', '.'));
        string.append(".xml");
        sourcePanel->getFunctionsDB()->CopyItemToFile(*i, string);
    }
    for (auto i = sourcePanel->getChosenFolders().begin(); i != sourcePanel->getChosenFolders().end(); i++)
    {
        sourcePanel->getFunctionsDB()->CopyFolder((*i)->first, sourcePanel->getCurrentFolder(), destinationPanel->getCurrentFolder());
    }
}

void Workspace::move()
{
    if (isLeftCurrent)
    {
        if (!leftPanel->getIsDB())
        {
            if (rightPanel->getIsDB())
            {
                moveFilesystemToDatabase(leftPanel, rightPanel);
            }
            else
            {
                moveFilesystemToFilesystem(leftPanel, rightPanel);
            }
        }
        else
        {
            if (rightPanel->getIsDB())
            {
                moveDatabaseToDatabase(leftPanel, rightPanel);
            }
            else
            {
                moveDatabaseToFilesystem(leftPanel, rightPanel);
            }
        }
    }
    else
    {
        if (!rightPanel->getIsDB())
        {
            if (leftPanel->getIsDB())
            {
                moveFilesystemToDatabase(rightPanel, leftPanel);
            }
            else
            {
                moveFilesystemToFilesystem(rightPanel, leftPanel);
            }
        }
        else
        {
            if (leftPanel->getIsDB())
            {
                moveDatabaseToDatabase(rightPanel, leftPanel);
            }
            else
            {
                moveDatabaseToFilesystem(rightPanel, leftPanel);
            }
        }
    }

    leftPanel->setSelectionMode(QAbstractItemView::NoSelection);
    rightPanel->setSelectionMode(QAbstractItemView::NoSelection);
    leftPanel->changeDirectory(leftPanel->rootIndex());
    rightPanel->changeDirectory(rightPanel->rootIndex());

    updatePanels();
}

void Workspace::moveFilesystemToDatabase(Panel* sourcePanel, Panel* destinationPanel)
{
    QModelIndexList& il = sourcePanel->getList();
    for (auto i = il.begin(); i != il.end(); i++)
    {
        destinationPanel->getFunctionsDB()->CopyFileToDB(sourcePanel->getFilesystem()->fileInfo(*i).absoluteFilePath(), destinationPanel->getCurrentFolder());
        sourcePanel->getFilesystem()->removeIndex(*i);
    }
    sourcePanel->getList().clear();
}

void Workspace::moveFilesystemToFilesystem(Panel* sourcePanel, Panel* destinationPanel)
{
    QModelIndexList& il = sourcePanel->getList();
    for (auto i = il.begin(); i != il.end(); i++)
    {
        sourcePanel->getFilesystem()->copyIndex(*i, destinationPanel->getPath());
        sourcePanel->getFilesystem()->removeIndex(*i);
    }
}

void Workspace::moveDatabaseToDatabase(Panel* sourcePanel, Panel* destinationPanel)
{
    copyDatabaseToDatabase(sourcePanel, destinationPanel);
    for (auto i = sourcePanel->getChosenItems().begin(); i != sourcePanel->getChosenItems().end(); i++)
    {
        sourcePanel->getFunctionsDB()->DeleteItem(*i, sourcePanel->getCurrentFolder());
    }
    for (auto i = sourcePanel->getChosenFolders().begin(); i != sourcePanel->getChosenFolders().end(); i++)
    {
        sourcePanel->getFunctionsDB()->DeleteFolder((*i)->first);
    }
}

void Workspace::moveDatabaseToFilesystem(Panel* sourcePanel, Panel* destinationPanel)
{
    for (auto i = sourcePanel->getChosenItems().begin(); i != sourcePanel->getChosenItems().end(); i++)
    {
        QString string = destinationPanel->getPath();
        string.append("/");
        string.append((*i)->name.replace(':', '.').replace('/', '.').replace('\\', '.'));
        string.append(".xml");
        sourcePanel->getFunctionsDB()->CopyItemToFile(*i, string);
        sourcePanel->getFunctionsDB()->DeleteItem(*i, sourcePanel->getCurrentFolder());
    }
    for (auto i = sourcePanel->getChosenFolders().begin(); i != sourcePanel->getChosenFolders().end(); i++)
    {
        sourcePanel->getFunctionsDB()->CopyFolder((*i)->first, sourcePanel->getCurrentFolder(), destinationPanel->getCurrentFolder());
        sourcePanel->getFunctionsDB()->DeleteFolder((*i)->first);
    }
}

void Workspace::createDir()
{
    if (isLeftCurrent && !leftPanel->getIsDB())
    {
        createDirFileSystem(leftPanel);
    }
    else if (!isLeftCurrent && !rightPanel->getIsDB())
    {
        createDirFileSystem(rightPanel);
    }
    else if (isLeftCurrent && leftPanel->getIsDB())
    {
        createDirDatabase(leftPanel);
    }
    else if (!isLeftCurrent && rightPanel->getIsDB())
    {
        createDirDatabase(rightPanel);
    }

    updatePanels();
}

void Workspace::createDirFileSystem(Panel* panel)
{
    QString newCatalog = QInputDialog::getText(0,
                                               tr("Новая папка"),
                                               tr("Введите название: "),
                                               QLineEdit::Normal,
                                               "");
    fileSystem->mkdir(panel->rootIndex(), newCatalog);
    updateFolder(isLeftCurrent, panel->getPath());
}

void Workspace::createDirDatabase(Panel* panel)
{
    QString newCatalog = QInputDialog::getText(0,
                                               tr("Новая папка"),
                                               tr("Введите название: "),
                                               QLineEdit::Normal,
                                               "");
    int new_id = panel->getFunctionsDB()->NewFolder(newCatalog);
    QStandardItem* id = new QStandardItem(QString::number(new_id));
    QStandardItem* name = new QStandardItem(newCatalog);
    name->setIcon(QIcon(":/icons/resources/folder.png"));
    panel->getDB()->setItem(panel->getFolders()->size() + panel->getItems()->size(), 2, id);
    panel->getDB()->setItem(panel->getFolders()->size() + panel->getItems()->size(), 0, name);
    folderinfo* fold = new folderinfo;
    fold->first = new_id;
    fold->second = newCatalog;
    panel->getFolders()->push_back(fold);
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

void Workspace::updateInfo(bool isLeft, bool isPlus, QModelIndex index)
{
    if (fileSystem->fileName(index) == "..")
        return;
    if (isLeft)
    {
        isLeftCurrent = true;
        if (!leftPanel->getIsDB())
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
        if (!rightPanel->getIsDB())
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
    qDebug() << leftPanel->getList();
    qDebug() << rightPanel->getList();
}

void Workspace::indexToString(bool isLeft, QModelIndex index)
{
    if (isLeft && leftPanel->getIsDB())
    {
        return;
    }
    else if (!isLeft && rightPanel->getIsDB())
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

void Workspace::comparePortraits()
{
    if (!leftPanel->getIsDB() || !rightPanel->getIsDB())
    {
        qDebug() << "В обоих панелях должна быть открыта база данных";
        return;
    }
    if (leftPanel->getChosenItems().empty() || rightPanel->getChosenItems().empty())
    {
        qDebug() << "В одной из панелей не выбрано ни одного элемента";
        return;
    }

    auto leftPortrait = *(leftPanel->getChosenItems().rbegin());
    auto rightPortrait = *(rightPanel->getChosenItems().rbegin());

    auto fullLeftPortrait = leftPanel->getFunctionsDB()->loadFromDB(leftPortrait->id);
    auto fullRightPortrait = rightPanel->getFunctionsDB()->loadFromDB(rightPortrait->id);

    calculateComparisonParameters(fullLeftPortrait->terms, fullRightPortrait->terms);
    calculateComparisonCircles();
    qDebug() << "Circle 1 radius " << comparisonResults.r1;
    qDebug() << "Circle 2 radius " << comparisonResults.r2;
    qDebug() << "Distance between circles " << comparisonResults.d;
    ipCompare = new IPCompare(comparisonResults);
    connect(ipCompare, &QObject::destroyed, this, &Workspace::handleWidgetDestroyed);
    ipCompare->show();
    widgetsList.append(ipCompare);
    clearComparisonResults();
}

void Workspace::killChildren() {
    foreach (QWidget* widget, widgetsList) {
        delete widget;
    }
    widgetsList.clear();
}

void Workspace::clearComparisonResults()
{
    comparisonResults.comm1 = 0.0;
    comparisonResults.comm2 = 0.0;
    comparisonResults.diff1 = 0.0;
    comparisonResults.diff2 = 0.0;
    comparisonResults.r1 = 0.0;
    comparisonResults.r2 = 0.0;
    comparisonResults.d = 0.0;
}


void Workspace::calculateComparisonParameters(const std::vector<TIPFullTermInfo*>& leftTerms,
                                            const std::vector<TIPFullTermInfo*>& rightTerms)
{
    double c1, c2, d1, d2;
    std::unordered_map<QString, double> leftTermWeights;
    std::unordered_map<QString, double> rightTermWeights;

    for (const auto& term : leftTerms) {
        leftTermWeights[term->term] = term->weight;
    }

    for (const auto& term : rightTerms) {
        rightTermWeights[term->term] = term->weight;
    }

    c1 = 0;
    d1 = 0;
    for (const auto& term : leftTerms) {
        if (rightTermWeights.find(term->term) != rightTermWeights.end()) {
            c1 += term->weight;
        } else {
            d1 += term->weight;
        }
    }

    c2 = 0;
    d2 = 0;
    for (const auto& term : rightTerms) {
        if (leftTermWeights.find(term->term) != leftTermWeights.end()) {
            c2 += term->weight;
        } else {
            d2 += term->weight;
        }
    }
    comparisonResults.comm1 = c1;
    comparisonResults.comm2 = c2;
    comparisonResults.diff1 = d1;
    comparisonResults.diff2 = d2;
}

void Workspace::calculateComparisonCircles()
{
    double c1 = comparisonResults.comm1;
    double c2 = comparisonResults.comm2;
    double d1 = comparisonResults.diff1;
    double d2 = comparisonResults.diff2;
    double r1 = 0, r2 = 0, rmax = 100, rmin = 0, d = 0, nc2 = 0, nd2 = 0;
    nc2 = c1;
    nd2 = (c2 > 0.0000000001) ? d2*c1/c2 : d2;
    if (c1 + d1 > nc2 + nd2)
    {
        r1 = rmax;
        rmin = r2 = sqrt(100. * 100 * (nc2 + nd2) / (c1 + d1));
        if (c1 > 0)
            d = rmax + 2*rmin * (0.5 - nc2 / (nc2 + nd2) );
        else
            d = rmin + rmax;
    }
    else
    {
        r2 = rmax;
        rmin = r1 = sqrt(100. * 100 * (c1 + d1) / (nc2 + nd2));
        if (c2 > 0)
            d = rmax  + 2*rmin * (0.5 - c1 / (c1 + d1));
        else
            d = rmin + rmax;
    }
    comparisonResults.r1 = r1;
    comparisonResults.r2 = r2;
    comparisonResults.d = d;
}

void Workspace::handleWidgetDestroyed(QObject *object)
{
    QWidget *widget = qobject_cast<QWidget*>(object);
    if (widget && widgetsList.contains(widget)) {
        widgetsList.removeOne(widget);
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
