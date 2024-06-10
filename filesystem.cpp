#include "filesystem.h"

FileSystem::FileSystem(QObject *parent)
    : QFileSystemModel{parent}
{
    setFilter(QDir::NoDot | QDir::AllEntries | QDir::System);
    setRootPath("");
    setReadOnly(false);
}

void FileSystem::copyFolder(QString sourceFolder, QString destFolder)
{
    QDir sourceDir(sourceFolder);
    if (!sourceDir.exists())
        return;
    QDir destDir(destFolder);
    if (!destDir.exists()) {
        destDir.mkdir(destFolder);
    }
    QStringList files = sourceDir.entryList(QDir::Files);
    for (int i = 0; i < files.count(); i++) {
        QString srcName = sourceFolder + "/" + files[i];
        QString destName = destFolder + "/" + files[i];
        QFile::copy(srcName, destName);
    }
    files.clear();
    files = sourceDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    for (int i = 0; i < files.count(); i++) {
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
    if (sourceFileInfo.absoluteFilePath() == destinationFileInfo.absoluteFilePath()) {
        return false;
    }

    if (sourceFileInfo.fileName() == "." || sourceFileInfo.fileName() == "..")
        return false;

    if (sourceFileInfo.isFile() && sourceFileInfo.isReadable()) {
        if (destinationFileInfo.isFile() && destinationFile.isWritable()) {
            QString newFileName = destinationFile.fileName();
            destinationFile.remove();
            sourceFile.copy(newFileName);
        } else if (destinationFileInfo.isDir() && destinationFileInfo.isWritable()) {
            QString newFileName = destinationFile.fileName();
            newFileName.append("/");
            newFileName.append(QFileInfo(sourceFile).fileName());
            sourceFile.copy(newFileName);
        }
    } else if (sourceFileInfo.isDir() && sourceFileInfo.isReadable()) {
        destinationFilePath.append("/");
        destinationFilePath.append(QFileInfo(sourceFile).fileName());
        this->copyFolder(sourceFilePath, destinationFilePath);
    }

    return true;
}

bool FileSystem::removeIndex(QModelIndex index)
{
    if (!this->isDir(index)) {
        this->remove(index);
        return true;
    } else {
        QString path = this->filePath(index);
        QDir dirToRemove(path);
        if (dirToRemove.removeRecursively()) {
            return true;
        } else {
            qCritical() << "Не удалось удалить папку " << path;
            return false;
        }
    }
}
