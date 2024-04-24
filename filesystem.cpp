#include "filesystem.h"
#include <QInputDialog>
#include <QDebug>

FileSystem::FileSystem(QObject *parent) :
    QFileSystemModel{parent}
{
    setFilter(QDir::NoDot | QDir::AllEntries | QDir::System);
    setRootPath("");
    setReadOnly(false);
}

void FileSystem::copyFolder(QString sourceFolder, QString destFolder)
{
    QDir sourceDir(sourceFolder);
    if(!sourceDir.exists())
        return;
    QDir destDir(destFolder);
    if(!destDir.exists())
    {
        destDir.mkdir(destFolder);
    }
    QStringList files = sourceDir.entryList(QDir::Files);
    for(int i = 0; i< files.count(); i++)
    {
        QString srcName = sourceFolder + "/" + files[i];
        QString destName = destFolder + "/" + files[i];
        QFile::copy(srcName, destName);
    }
    files.clear();
    files = sourceDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    for(int i = 0; i< files.count(); i++)
    {
        QString srcName = sourceFolder + "/" + files[i];
        QString destName = destFolder + "/" + files[i];
        copyFolder(srcName, destName);
    }
}

bool FileSystem::copyIndex(QModelIndex index, QString destinationFilePath)
{
    QString sourceFilePath = this->fileInfo(index).absoluteFilePath();
    QFile sourceFile(sourceFilePath);
    QFile destinationFile(destinationFilePath);
    QFileInfo sourceFileInfo(sourceFile);
    QFileInfo destinationFileInfo(destinationFile);
    if (sourceFileInfo.absoluteFilePath() == destinationFileInfo.absoluteFilePath())
    {
        return false;
    }

    if(sourceFileInfo.fileName() == "." || sourceFileInfo.fileName() == "..")
       return false;

    if(sourceFileInfo.isFile() && sourceFileInfo.isReadable())
    {
        if(destinationFileInfo.isFile() && destinationFile.isWritable())
        {
            QString newFileName = destinationFile.fileName();
            destinationFile.remove();
            sourceFile.copy(newFileName);
        }
        else if(destinationFileInfo.isDir() && destinationFileInfo.isWritable())
        {
            QString newFileName = destinationFile.fileName();
            newFileName.append("/");
            newFileName.append(QFileInfo(sourceFile).fileName());
            sourceFile.copy(newFileName);
        }
    }
    else if(sourceFileInfo.isDir() && sourceFileInfo.isReadable())
    {
        destinationFilePath.append("/");
        destinationFilePath.append(QFileInfo(sourceFile).fileName());
        this->copyFolder(sourceFilePath, destinationFilePath);
    }

    return true;
}

bool FileSystem::removeIndex(QModelIndex index)
{
    if (!this->isDir(index))
    {
        this->remove(index);
        return true;
    }
    else
    {
        // Set the root directory to the directory containing the index
        QString rootPath = this->filePath(index);
        QDir rootDir(rootPath);

        QDir dir;
        dir.setPath(this->fileInfo(index).absoluteFilePath());

        removeFolder(dir, rootDir);
        return true;
    }
}

bool FileSystem::removeFolder(QDir dir, const QDir &rootDir)
{
    // Проверяем, находится ли папка в корневой директории
    if (!dir.absolutePath().startsWith(rootDir.absolutePath())) {
        qDebug() << "dir " << dir.absolutePath() << " is not in root Dir " << rootDir.absolutePath();
        return false;
    }

    //Получаем список каталогов
    QStringList lstDirs = dir.entryList(QDir::Dirs |
                    QDir::AllDirs |
                    QDir::NoDotAndDotDot);
    //Получаем список файлов
    QStringList lstFiles = dir.entryList(QDir::Files);

    // Удаляем файлы
    foreach (QString entry, lstFiles)
    {
        QString entryAbsPath = dir.absolutePath() + "/" + entry;
        QFile::setPermissions(entryAbsPath, QFile::ReadOwner | QFile::WriteOwner);
        if (!QFile::remove(entryAbsPath)) {
            qDebug() << "Failed deleting file " << entryAbsPath << " from directory " << dir;
            return false;
        }
    }

    // Рекурсивно обрабатываем поддиректории
    foreach (QString entry, lstDirs)
    {
        QString entryAbsPath = dir.absolutePath() + "/" + entry;
        QDir dr(entryAbsPath);
        if (!removeFolder(dr, rootDir)) {
            qDebug() << "Failed to delete subdirectory " << dir;
            return false;
        }
    }

    // Удаляем саму директорию
    if (!QDir().rmdir(dir.absolutePath()))
    {
        qDebug() << "Failed to delete directory " << dir;
        return false;
    }

    return true;
}

bool FileSystem::renameIndex(QModelIndex index, const QString &newPath)
{
    if (!index.isValid())
      return false;

    // Get the current file path
    QString currentFilePath = filePath(index);

    // Check if the file exists
    if (!QFile::exists(currentFilePath))
      return false;

    // Perform the renaming operation
    if (QFile::rename(currentFilePath, newPath)) {
      return true;
    } else {
      return false;
    }
}

long long int FileSystem::getFolderSize(QString path)
{
    QFileInfo info(path);
    if (!info.isDir())
        return info.size() / 1024;
    QDir currentFolder( path );
    long long int size = 0;
    currentFolder.setFilter( QDir::Dirs | QDir::Files | QDir::NoSymLinks );
    currentFolder.setSorting( QDir::Name );

    QFileInfoList folderitems( currentFolder.entryInfoList() );

    foreach ( QFileInfo i, folderitems )
    {
        QString iname( i.fileName() );
        if ( iname == "." || iname == ".." || iname.isEmpty() )
            continue;
        if ( !i.isDir())
            size += i.size();
    }
    return size / 1024;
}

