#ifndef TIPDBSHELL_H
#define TIPDBSHELL_H

#include <vector>
#include <QString>
#include <QTime>
#include <QModelIndex>
#include <QFileSystemModel>
#include <QMessageBox>
#include <QDateTime>

#include <QtSql/QtSql>

using namespace std;

class ViewIP;
class IPPortrait;

struct TIPInfo
{
    long id;
    QString name;
    long sizeInBytes;
    long sizeInTerms;
    QDateTime creationTime;
    QString ownerName;
    double compData1;
    double compData2;
    double compData3;
};

struct TIPFullTermInfo
{
    long id;
    QString term;
    double weight;
    double occurences;
    long times;
    long size; // только для шинглов
    long shows; // только для шинглов

    TIPFullTermInfo() {}
    TIPFullTermInfo(long _id, QString _term, double _weight, double _occurences, long _times, long _size, long _shows):
        id(_id), term(_term), weight(_weight), occurences(_occurences), times(_times), size(_size), shows(_shows) {}
};

struct TIPFullInfo
{
    long id;
    long object_id;
    long aspect_id;
    long attribute_id;
    long norm_id;
    long creator_id;
    QString comment;
    QString ids_list;
    QDateTime date_add;
    QDateTime req_from;
    QDateTime req_till;
    QString source_text;
    long words;
    QString abbreviations;
    long shingle_count;
    char *source_lo;
    char *abbr_lo;

    vector<TIPFullTermInfo*> terms;
    vector<TIPFullTermInfo*> shingles;

    TIPFullInfo() {}
    TIPFullInfo(IPPortrait *ip);
    ~TIPFullInfo() {}
};

typedef int folderid;
typedef pair<folderid, QString> folderinfo;

class TIPDBShell
{
    QSqlDatabase *db;
    folderid curFolder;
    ViewIP *w;

    folderid getFolderIdByName(QString newFolder);
    QString getFolderNameById(folderid id);
    bool loadFromFile(QString filename, TIPInfo *info, TIPFullInfo *full);
    bool saveToDB(TIPFullInfo *full);
    bool saveToFile(TIPFullInfo *full, QString filename);

public:
    bool Init(QString instance = "");
    bool Init(QString shost, QString sdbname, int iport, QString suser, QString spass);

    QSqlDatabase *getDataBase();
    // Переместил из private
    TIPFullInfo *loadFromDB(int id);

    folderid ChangeFolder(folderid id = 0, QString newFolder = "");
    folderinfo GetCurrentFolder();
    bool GetFolderContents(folderid id, vector<TIPInfo*> &Items, vector<folderinfo*> &Folders);

    bool CopyItemWithinDatabase(TIPInfo *item, folderid source, folderid destination);
    bool CopyItemBetweenDatabases(TIPInfo *item, QSqlDatabase *sourceDB, QSqlDatabase *destinationDB, folderid destination);
    bool MoveItem(TIPInfo *item, folderid source, folderid destination);
    bool DeleteItem(TIPInfo *item, folderid source);
    folderid NewFolder(QString name, folderid parent = 0);
    bool CopyFolder(folderid id, folderid source, folderid destination);
    bool MoveFolder(folderid id, folderid source, folderid destination);
    bool DeleteFolder(folderid source);
    bool RenameFolder(folderid id, const QString &newName);
    bool RenameItem(TIPInfo *item, const QString &newName);
    bool OpenItem(TIPInfo *item);

    TIPInfo *CopyFileToDB(QString filename, folderid destination);
    bool CopyItemToFile(TIPInfo *item, QString filename);

    float GetOperationProgress();

    bool CompareItems(TIPFullInfo *item1, TIPFullInfo *item2);
    bool CompareItemsToFile(QString filename);

private:
    folderid GetNextSequenceValue(QSqlQuery &query, const QString &sequenceName);
};

#endif // TIPDBSHELL_H
