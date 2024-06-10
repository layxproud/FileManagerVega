#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QMainWindow>
#include <QString>

class FileSystem : public QFileSystemModel
{
public:
    FileSystem(QObject *parent = 0);
    bool copyIndex(QModelIndex index, QString destination);
    void copyFolder(QString sourceFolder, QString destFolder);
    bool removeIndex(QModelIndex index);
};

#endif // FILESYSTEM_H
