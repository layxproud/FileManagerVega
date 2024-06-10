#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QMainWindow>
#include <QString>

class DeletionWorker : public QObject
{
    Q_OBJECT

public:
    DeletionWorker(QFileSystemModel *model, const QModelIndexList &indexes)
        : model(model)
        , indexes(indexes)
    {}

signals:
    void progress(int value);
    void finished();

public slots:
    void process()
    {
        int total = indexes.size();
        for (int i = 0; i < total; ++i) {
            removeIndex(indexes[i]);
            emit progress(static_cast<int>((i + 1) / static_cast<float>(total) * 100));
        }
        emit finished();
    }

private:
    QFileSystemModel *model;
    QModelIndexList indexes;

    bool removeIndex(const QModelIndex &index)
    {
        if (!model->isDir(index)) {
            model->remove(index);
            return true;
        } else {
            QString path = model->filePath(index);
            QDir dirToRemove(path);
            if (dirToRemove.removeRecursively()) {
                return true;
            } else {
                qCritical() << "Failed to remove directory " << path;
                return false;
            }
        }
    }
};

class FileSystem : public QFileSystemModel
{
public:
    FileSystem(QObject *parent = 0);
    bool copyIndex(QModelIndex index, QString destination);
    void copyFolder(QString sourceFolder, QString destFolder);
    bool removeIndex(QModelIndex index);
};

#endif // FILESYSTEM_H
