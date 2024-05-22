#include "tipdbshell.h"
#include "inifile.h"
#include "tinyxml2.h"
#include "trmlshell.h"
#include "viewip.h"
#include <QDebug>
#include <QMessageBox>

bool start = true;
folderid count_folders = 1;
int count_item_id = 0;

bool TIPDBShell::Init(QString instance)
{
    TIniFile *ini = new TIniFile("db.ini");
    if (!ini->IsExist()) {
        // Unable to open ini-file
        qDebug()
            << "? ***ERROR: initDb(): Can`t open ini-file for PZ database connection parameters!";
        return false;
    }

    char *host = strdup(ini->ReadString(instance.toLocal8Bit().data(), "host", "localhost").c_str()),
         *dbname = strdup(ini->ReadString(instance.toLocal8Bit().data(), "db", "ip3").c_str()),
         *user = strdup(ini->ReadString(instance.toLocal8Bit().data(), "user", "postgres").c_str()),
         *pass = strdup(ini->ReadString(instance.toLocal8Bit().data(), "pass", "123456").c_str());
    int port = ini->ReadInteger(instance.toLocal8Bit().data(), "port", 5432);

    db = new QSqlDatabase(QSqlDatabase::database(instance));
    if (!db->isValid())
        db = new QSqlDatabase(QSqlDatabase::addDatabase("QPSQL", instance));
    if (!db->isValid()) {
        // Unable to connect DB
        qDebug() << "? ***ERROR: initDb(): Can`t use or create connection";
        free(host);
        free(dbname);
        free(user);
        free(pass);
        return false;
    }

    db->setHostName(host);
    db->setPort(port);
    db->setDatabaseName(dbname);
    db->setUserName(user);
    db->setPassword(pass);

    bool ok = db->open();
    if (!ok) {
        QSqlError res = db->lastError();
        db = NULL;
        qDebug() << QString("? ***ERROR: initDb(): Can`t connect to PZ database: %1@%2:%3/%4")
                        .arg(user)
                        .arg(host)
                        .arg(port)
                        .arg(dbname);
        // LogPrintf("? ***ERROR: initDb(): %s", res.text().toStdString().c_str());
        free(host);
        free(dbname);
        free(user);
        free(pass);
        return false;
    }
    // LogPrintf("initDb(): Connect to PZ database: %s@%s/%s", user, host, dbname);

    free(host);
    free(dbname);
    free(user);
    free(pass);

    curFolder = 1;

    if (!lemmatizer)
        lemmatizer = new TRMLShell;
    return true;
}

bool TIPDBShell::Init(QString shost, QString sdbname, int iport, QString suser, QString spass)
{
    const char *host = shost.toLocal8Bit().data(), *dbname = sdbname.toLocal8Bit().data(),
               *user = suser.toLocal8Bit().data(), *pass = spass.toLocal8Bit().data();

    db = new QSqlDatabase(QSqlDatabase::database(shost + "-" + sdbname + "-" + iport + "-" + suser));
    if (!db->isValid())
        db = new QSqlDatabase(
            QSqlDatabase::addDatabase("QPSQL", shost + "-" + sdbname + "-" + iport + "-" + suser));
    if (!db->isValid()) {
        // Unable to connect DB
        // LogPrintf("? ***ERROR: initDb(): Can`t use or create connection: ", dbsectionname.toStdString().c_str());
        return false;
    }

    db->setHostName(host);
    db->setPort(iport);
    db->setDatabaseName(dbname);
    db->setUserName(user);
    db->setPassword(pass);

    bool ok = db->open();
    if (!ok) {
        QSqlError res = db->lastError();
        db = NULL;
        // LogPrintf("? ***ERROR: initDb(): Can`t connect to PZ database: %s@%s/%s", user, host, dbname);
        // LogPrintf("? ***ERROR: initDb(): %s", res.text().toStdString().c_str());
        delete host;
        delete dbname;
        delete user;
        delete pass;
        return false;
    }
    // LogPrintf("initDb(): Connect to PZ database: %s@%s/%s", user, host, dbname);

    delete host;
    delete dbname;
    delete user;
    delete pass;

    curFolder = 1;
    return true;
}

QSqlDatabase *TIPDBShell::getDataBase()
{
    if (!db)
        return nullptr;
    return db;
}

folderid TIPDBShell::getFolderIdByName(QString newFolder)
{
    QStringList sl = newFolder.split('/', Qt::SkipEmptyParts);

    folderid cur = 1;
    int i = 0;
    while (i < sl.size()) {
        QSqlQuery dbq(*db);
        dbq.exec(QString("select id from in_collection where title = \'%1\' and parent_id = %2")
                     .arg(sl[i])
                     .arg(cur));

        if (!dbq.first() || !dbq.seek(0, false)) {
            QSqlError res = db->lastError();
            // Unable to find such folder
            // LogPrintf("? ERROR(%d): %s", res.number(), res.text().toStdString().c_str());
            // LogPrintf("? ..... at query: %s", query.toStdString().c_str());
            return 0;
        }
        cur = dbq.value(0).toInt();
        i++;
    }

    return cur;
}

QString TIPDBShell::getFolderNameById(folderid id)
{
    if (!id)
        return "";
    if (id == 1)
        return "/";

    folderid cur = id;
    QString str = "";
    int i = 0;
    while (cur != 1 && i < 30) {
        QSqlQuery dbq(*db);
        dbq.exec(QString("select parent_id, title from in_collection where id = %1").arg(cur));

        if (!dbq.first() || !dbq.seek(0, false)) {
            QSqlError res = db->lastError();
            // Unable to find such folder
            // LogPrintf("? ERROR(%d): %s", res.number(), res.text().toStdString().c_str());
            // LogPrintf("? ..... at query: %s", query.toStdString().c_str());
            return "";
        }
        cur = dbq.value(0).toInt();
        str = QString("/") + dbq.value(1).toString() + str;
        i++;
    }

    return str;
}

folderid TIPDBShell::ChangeFolder(folderid id, QString newFolder)
{
    if (id == 0 && newFolder == "")
        id = 1;

    if (curFolder == id)
        return curFolder;

    if (id != 0) {
        QSqlQuery dbq(*db);
        dbq.exec(QString("select id from in_collection where id = %1").arg(id));

        if (!dbq.first() || !dbq.seek(0, false)) {
            QSqlError res = db->lastError();
            // Unable to find such folder
            // LogPrintf("? ERROR(%d): %s", res.number(), res.text().toStdString().c_str());
            // LogPrintf("? ..... at query: %s", query.toStdString().c_str());
            return 0;
        }
        return (curFolder = dbq.value(0).toInt());
    } else {
        folderid newid = getFolderIdByName(newFolder);
        if (newid)
            curFolder = newid;
        return newid;
    }
}

folderinfo TIPDBShell::GetCurrentFolder()
{
    pair<folderid, QString> ret = pair<folderid, QString>(curFolder, getFolderNameById(curFolder));
    return ret;
}

bool TIPDBShell::GetFolderContents(
    folderid id, vector<TIPInfo *> &Items, vector<folderinfo *> &Folders)
{
    if (!id)
        id = 1;
    QSqlQuery dbq(*db);
    dbq.exec(
        QString("select id, title from in_collection where parent_id = %1 order by title").arg(id));

    if (!dbq.first() || !dbq.seek(0, false)) {
        /*        QSqlError res = db->lastError();
            // Unable to find such folder
            // LogPrintf("? ERROR(%d): %s", res.number(), res.text().toStdString().c_str());
            // LogPrintf("? ..... at query: %s", query.toStdString().c_str());
            return false;
    */
    } else {
        do {
            folderinfo *fi = new folderinfo;
            fi->first = dbq.value(0).toInt();
            fi->second = dbq.value(1).toString(); // getFolderNameById(fi->first);
            Folders.push_back(fi);
        } while (dbq.next());
    }

    dbq.exec(QString("select dc.doc_id, ip.comment, ip.date_add, (ip.words + ip.shingles) sizet, "
                     "ip.creator_id "
                     " from in_doc_collections dc inner join ip_portrait ip on dc.doc_id = ip.id "
                     "where dc.coll_id = %1 "
                     "order by ip.id")
                 .arg(id));
    if (!dbq.first() || !dbq.seek(0, false)) {
        /*
    QSqlError res = db->lastError();
    // Unable to find such folder
    // LogPrintf("? ERROR(%d): %s", res.number(), res.text().toStdString().c_str());
    // LogPrintf("? ..... at query: %s", query.toStdString().c_str());
    return false;*/
    } else {
        do {
            TIPInfo *item = new TIPInfo;
            item->id = dbq.value(0).toInt();
            item->name = dbq.value(1).toString();
            item->creationTime = dbq.value(2).toDateTime();
            item->sizeInTerms = dbq.value(3).toInt();
            item->ownerName = QString("%1").arg(dbq.value(4).toInt()); // change to username

            // not filled in
            item->sizeInBytes = 0;
            item->compData1 = 0;
            item->compData2 = 0;
            item->compData3 = 0;

            Items.push_back(item);
        } while (dbq.next());
    }
    return true;
}

bool TIPDBShell::CopyItemWithinDatabase(TIPInfo *item, folderid source, folderid destination)
{
    if (!item || !item->id || !destination)
        return false; // Wrong input data

    QSqlQuery dbq(*db);
    dbq.exec("select nextval(\'ip_portret_id_seq\')");

    if (!dbq.first() || !dbq.seek(0, false)) {
        QSqlError res = db->lastError();
        // Unable to get new portrait id
        // LogPrintf("? ERROR(%d): %s", res.number(), res.text().toStdString().c_str());
        // LogPrintf("? ..... at query: %s", query.toStdString().c_str());
        return false;
    }

    folderid newid = dbq.value(0).toInt();

    bool ret = dbq.exec(
        QString(
            "insert into ip_portrait (id, object_id, aspect_id, attribute_id, norm_id, creator_id, "
            "comment, ids_list, date_add, req_from, req_till, source_text, words, abbreviations, "
            "shingles) "
            "select %1 id, object_id, aspect_id, attribute_id, norm_id, creator_id, comment, "
            "ids_list, date_add, "
            "req_from, req_till, source_text, words, abbreviations, shingles "
            "from ip_portrait where id = %2;")
            .arg(newid)
            .arg(item->id));
    if (!ret) {
        QSqlError res = db->lastError();
        // Unable to copy portrait parameters
        // LogPrintf("? ERROR(%d): %s", res.number(), res.text().toStdString().c_str());
        // LogPrintf("? ..... at query: %s", query.toStdString().c_str());
        return false;
    }

    ret = dbq.exec(
        QString("insert into ip_term (id, ip_id, term, weight, occurences, times) "
                "select nextval('ipterm_id_seq'), %1 ip_id, term, weight, occurences, times "
                "from ip_term where ip_id = %2;")
            .arg(newid)
            .arg(item->id));
    if (!ret) {
        QSqlError res = db->lastError();
        // Unable to copy portrait terms
        // LogPrintf("? ERROR(%d): %s", res.number(), res.text().toStdString().c_str());
        // LogPrintf("? ..... at query: %s", query.toStdString().c_str());
        return false;
    }

    ret = dbq.exec(QString("insert into ip_term_shingles (id, ip_id, term, weight, occurences, "
                           "times, size, shows) "
                           "select nextval('ipterm_id_seq'), %1 ip_id, term, weight, occurences, "
                           "times, size, shows "
                           "from ip_term_shingles where ip_id = %2;")
                       .arg(newid)
                       .arg(item->id));
    if (!ret) {
        QSqlError res = db->lastError();
        // Unable to copy portrait shingles
        // LogPrintf("? ERROR(%d): %s", res.number(), res.text().toStdString().c_str());
        // LogPrintf("? ..... at query: %s", query.toStdString().c_str());
        return false;
    }

    ret = dbq.exec(QString("insert into in_doc_collections (doc_id, coll_id) values (%1, %2);")
                       .arg(newid)
                       .arg(destination));
    if (!ret) {
        QSqlError res = db->lastError();
        // Unable to copy portrait shingles
        // LogPrintf("? ERROR(%d): %s", res.number(), res.text().toStdString().c_str());
        // LogPrintf("? ..... at query: %s", query.toStdString().c_str());
        return false;
    }

    return true;
}

bool TIPDBShell::CopyItemBetweenDatabases(
    TIPInfo *item, QSqlDatabase *sourceDB, QSqlDatabase *destinationDB, folderid destination)
{
    if (!item || !item->id || !destination)
        return false;

    QSqlQuery sourceQuery(*sourceDB);
    QSqlQuery destinationQuery(*destinationDB);

    sourceDB->transaction();
    destinationDB->transaction();

    try {
        destinationQuery.exec("SELECT nextval('ip_portret_id_seq')");
        if (destinationQuery.first() && destinationQuery.seek(0, false)) {
            folderid newid = destinationQuery.value(0).toInt();

            // 1. Copy data from ip_portrait
            sourceQuery.prepare("SELECT * FROM ip_portrait WHERE id = :id");
            sourceQuery.bindValue(":id", QVariant::fromValue(item->id));
            if (sourceQuery.exec()) {
                while (sourceQuery.next()) {
                    destinationQuery.prepare(
                        "INSERT INTO ip_portrait (id, object_id, aspect_id, attribute_id, norm_id, "
                        "creator_id, "
                        "comment, ids_list, date_add, req_from, req_till, source_text, words, "
                        "abbreviations, shingles) "
                        "VALUES (:id, :object_id, :aspect_id, :attribute_id, :norm_id, "
                        ":creator_id, "
                        ":comment, :ids_list, :date_add, :req_from, :req_till, :source_text, "
                        ":words, :abbreviations, :shingles)");
                    destinationQuery.bindValue(":id", QVariant::fromValue(newid));
                    destinationQuery.bindValue(":object_id", sourceQuery.value("object_id"));
                    destinationQuery.bindValue(":aspect_id", sourceQuery.value("aspect_id"));
                    destinationQuery.bindValue(":attribute_id", sourceQuery.value("attribute_id"));
                    destinationQuery.bindValue(":norm_id", sourceQuery.value("norm_id"));
                    destinationQuery.bindValue(":creator_id", sourceQuery.value("creator_id"));
                    destinationQuery.bindValue(":comment", sourceQuery.value("comment"));
                    destinationQuery.bindValue(":ids_list", sourceQuery.value("ids_list"));
                    destinationQuery.bindValue(":date_add", sourceQuery.value("date_add"));
                    destinationQuery.bindValue(":req_from", sourceQuery.value("req_from"));
                    destinationQuery.bindValue(":req_till", sourceQuery.value("req_till"));
                    destinationQuery.bindValue(":source_text", sourceQuery.value("source_text"));
                    destinationQuery.bindValue(":words", sourceQuery.value("words"));
                    destinationQuery.bindValue(":abbreviations", sourceQuery.value("abbreviations"));
                    destinationQuery.bindValue(":shingles", sourceQuery.value("shingles"));

                    if (!destinationQuery.exec()) {
                        throw std::runtime_error("Failed to insert into ip_portrait");
                    }
                }
            } else {
                throw std::runtime_error("Failed to fetch data from ip_portrait in sourceQuery");
            }

            // 2. Copy data from ip_term
            sourceQuery.prepare("SELECT * FROM ip_term WHERE ip_id = :id");
            sourceQuery.bindValue(":id", QVariant::fromValue(item->id));

            if (sourceQuery.exec()) {
                while (sourceQuery.next()) {
                    destinationQuery.prepare(
                        "INSERT INTO ip_term (id, ip_id, term, weight, occurences, times) "
                        "VALUES (nextval('ipterm_id_seq'), :ip_id, :term, :weight, :occurences, "
                        ":times)");
                    destinationQuery.bindValue(":ip_id", QVariant::fromValue(newid));
                    destinationQuery.bindValue(":term", sourceQuery.value("term"));
                    destinationQuery.bindValue(":weight", sourceQuery.value("weight"));
                    destinationQuery.bindValue(":occurences", sourceQuery.value("occurences"));
                    destinationQuery.bindValue(":times", sourceQuery.value("times"));

                    if (!destinationQuery.exec()) {
                        throw std::runtime_error("Failed to insert into ip_term");
                    }
                }
            } else {
                throw std::runtime_error("Failed to fetch data from ip_term in sourceQuery");
            }

            // 3. Copy data from ip_term_shingles
            sourceQuery.prepare("SELECT * FROM ip_term_shingles WHERE ip_id = :id");
            sourceQuery.bindValue(":id", QVariant::fromValue(item->id));

            if (sourceQuery.exec()) {
                while (sourceQuery.next()) {
                    destinationQuery.prepare("INSERT INTO ip_term_shingles (id, ip_id, term, "
                                             "weight, occurences, times, size, shows) "
                                             "VALUES (nextval('ipterm_id_seq'), :ip_id, :term, "
                                             ":weight, :occurences, :times, :size, :shows)");
                    destinationQuery.bindValue(":ip_id", QVariant::fromValue(newid));
                    destinationQuery.bindValue(":term", sourceQuery.value("term"));
                    destinationQuery.bindValue(":weight", sourceQuery.value("weight"));
                    destinationQuery.bindValue(":occurences", sourceQuery.value("occurences"));
                    destinationQuery.bindValue(":times", sourceQuery.value("times"));
                    destinationQuery.bindValue(":size", sourceQuery.value("size"));
                    destinationQuery.bindValue(":shows", sourceQuery.value("shows"));

                    if (!destinationQuery.exec()) {
                        throw std::runtime_error("Failed to insert into ip_term_shingles");
                    }
                }
            } else {
                throw std::runtime_error(
                    "Failed to fetch data from ip_term_shingles in sourceQuery");
            }

            // 4. Copy data to in_doc_collections
            destinationQuery.prepare(
                "INSERT INTO in_doc_collections (doc_id, coll_id) VALUES (:doc_id, :coll_id)");
            destinationQuery.bindValue(":doc_id", QVariant::fromValue(newid));
            destinationQuery.bindValue(":coll_id", QVariant::fromValue(destination));

            if (!destinationQuery.exec()) {
                throw std::runtime_error("Failed to insert into in_doc_collections");
            }

            // 5. Commit transactions
            sourceDB->commit();
            destinationDB->commit();
        }
    } catch (const std::exception &e) {
        sourceDB->rollback();
        destinationDB->rollback();

        qDebug() << "Error: " << e.what();
        return false;
    }

    return true;
}

bool TIPDBShell::DeleteItem(TIPInfo *item, folderid source)
{
    if (!item || !item->id)
        return false; // Wrong input data

    if (!source)
        source = curFolder;

    QSqlQuery dbq(*db);

    bool ret = dbq.exec(
        QString("select count(*) from in_doc_collections where doc_id = %1 and coll_id <> %2;")
            .arg(item->id)
            .arg(source));
    if (!dbq.first() || !dbq.seek(0, false)) {
        QSqlError res = db->lastError();
        // Unable to copy portrait shingles
        // LogPrintf("? ERROR(%d): %s", res.number(), res.text().toStdString().c_str());
        // LogPrintf("? ..... at query: %s", query.toStdString().c_str());
        return false;
    }
    int RefCount = dbq.value(0).toInt();
    if (RefCount < 1
        && QMessageBox::question(
               0,
               "Подтверждение",
               "Удаляется последняя копия объекта из БД. Продолжить?",
               QMessageBox::Yes | QMessageBox::No,
               QMessageBox::No)
               == QMessageBox::No)
        return false;

    ret = dbq.exec(QString("delete from in_doc_collections where doc_id = %1 and coll_id = %2;")
                       .arg(item->id)
                       .arg(source));
    if (!ret) {
        QSqlError res = db->lastError();
        // Unable to copy portrait shingles
        // LogPrintf("? ERROR(%d): %s", res.number(), res.text().toStdString().c_str());
        // LogPrintf("? ..... at query: %s", query.toStdString().c_str());
        return false;
    }

    ret = dbq.exec(QString("delete from ip_portrait where id = %1;").arg(item->id));
    if (!ret) {
        QSqlError res = db->lastError();
        // Unable to copy portrait parameters
        // LogPrintf("? ERROR(%d): %s", res.number(), res.text().toStdString().c_str());
        // LogPrintf("? ..... at query: %s", query.toStdString().c_str());
        return false;
    }

    ret = dbq.exec(QString("delete from ip_term where ip_id = %1;").arg(item->id));
    if (!ret) {
        QSqlError res = db->lastError();
        // Unable to copy portrait terms
        // LogPrintf("? ERROR(%d): %s", res.number(), res.text().toStdString().c_str());
        // LogPrintf("? ..... at query: %s", query.toStdString().c_str());
        return false;
    }

    ret = dbq.exec(QString("delete from ip_term_shingles where ip_id = %1;").arg(item->id));
    if (!ret) {
        QSqlError res = db->lastError();
        // Unable to copy portrait shingles
        // LogPrintf("? ERROR(%d): %s", res.number(), res.text().toStdString().c_str());
        // LogPrintf("? ..... at query: %s", query.toStdString().c_str());
        return false;
    }

    return true;
}

folderid TIPDBShell::NewFolder(QString name, folderid parent)
{
    if (!parent)
        parent = curFolder;

    QSqlQuery dbq(*db);
    dbq.exec("select nextval(\'in_collection_id_seq\')");

    if (!dbq.first() || !dbq.seek(0, false)) {
        QSqlError res = db->lastError();
        // Unable to get next folder id
        // LogPrintf("? ERROR(%d): %s", res.number(), res.text().toStdString().c_str());
        // LogPrintf("? ..... at query: %s", query.toStdString().c_str());
        return 0;
    }

    folderid newid = dbq.value(0).toInt();
    bool ret = dbq.exec(
        QString("insert into in_collection (id, title, parent_id) values(%1, \'%2\', %3)")
            .arg(newid)
            .arg(name)
            .arg(parent));

    if (!ret) {
        QSqlError res = db->lastError();
        // Error running insert auery
        // LogPrintf("? ERROR(%d): %s", res.number(), res.text().toStdString().c_str());
        // LogPrintf("? ..... at query: %s", query.toStdString().c_str());
        return false;
    }

    return newid;
}

bool TIPDBShell::CopyFolder(folderid id, folderid source, folderid destination)
{
    if (id <= 1 || !destination)
        return false;

    QSqlQuery dbq(*db);
    dbq.exec(QString("select title, parent from in_collection where id = %1").arg(id));
    if (!dbq.first() || !dbq.seek(0, false)) {
        QSqlError res = db->lastError();
        // Unable to get folder info
        // LogPrintf("? ERROR(%d): %s", res.number(), res.text().toStdString().c_str());
        // LogPrintf("? ..... at query: %s", query.toStdString().c_str());
        return false;
    }

    QString title = dbq.value(0).toString();
    folderid parent = dbq.value(1).toInt();
    if (parent == destination)
        return false;

    dbq.exec("select nextval(\'in_collection_id_seq\'");
    if (!dbq.first() || !dbq.seek(0, false)) {
        QSqlError res = db->lastError();
        // Unable to get next folder id
        // LogPrintf("? ERROR(%d): %s", res.number(), res.text().toStdString().c_str());
        // LogPrintf("? ..... at query: %s", query.toStdString().c_str());
        return false;
    }

    folderid newid = dbq.value(0).toInt();

    bool ret = dbq.exec(
        QString("insert tnto in_collection (id, title, parent_id) values(%1, \'%2\', %3)")
            .arg(newid)
            .arg(title)
            .arg(parent));
    if (!ret) {
        QSqlError res = db->lastError();
        // Error running insert query
        // LogPrintf("? ERROR(%d): %s", res.number(), res.text().toStdString().c_str());
        // LogPrintf("? ..... at query: %s", query.toStdString().c_str());
        return false;
    }

    ret = dbq.exec(
        QString("insert into in_doc_collections (coll_id, doc_id) select %1 coll_id, doc_id from "
                "in_doc_collections where coll_id = %2")
            .arg(newid)
            .arg(id));
    if (!ret) {
        QSqlError res = db->lastError();
        // Error copying folder contents
        // LogPrintf("? ERROR(%d): %s", res.number(), res.text().toStdString().c_str());
        // LogPrintf("? ..... at query: %s", query.toStdString().c_str());
        return false;
    }

    return true;
}

bool TIPDBShell::DeleteFolder(folderid source)
{
    if (source <= 1)
        return false;

    QSqlQuery dbq(*db);
    bool ret = dbq.exec(QString("delete from in_collection where id = %1").arg(source));
    if (!ret) {
        QSqlError res = db->lastError();
        // Error deleting folder
        // LogPrintf("? ERROR(%d): %s", res.number(), res.text().toStdString().c_str());
        // LogPrintf("? ..... at query: %s", query.toStdString().c_str());
        return false;
    }

    ret = dbq.exec(QString("delete from in_doc_collections where coll_id = %1").arg(source));
    if (!ret) {
        QSqlError res = db->lastError();
        // Error deleting folder contents
        // LogPrintf("? ERROR(%d): %s", res.number(), res.text().toStdString().c_str());
        // LogPrintf("? ..... at query: %s", query.toStdString().c_str());
        return false;
    }

    return true;
}

bool TIPDBShell::RenameFolder(folderid id, const QString &newName)
{
    if (id <= 1)
        return false;

    QSqlQuery dbq(*db);
    QString query = QString("UPDATE in_collection SET title = :newName WHERE id = :id");
    dbq.prepare(query);
    dbq.bindValue(":newName", newName);
    dbq.bindValue(":id", id);

    bool ret = dbq.exec();

    if (!ret) {
        QSqlError res = db->lastError();
        // Error updating folder title
        // Log error if needed
        return false;
    }

    return true;
}

bool TIPDBShell::RenameItem(TIPInfo *item, const QString &newName)
{
    if (!item || !item->id)
        return false; // Wrong input data

    QSqlQuery dbq(*db);
    QString query = QString("UPDATE ip_portrait SET comment = :newName WHERE id = :id");
    dbq.prepare(query);
    dbq.bindValue(":newName", newName);
    dbq.bindValue(":id", QVariant::fromValue(item->id));

    bool ret = dbq.exec();

    if (!ret) {
        QSqlError res = db->lastError();
        // Error updating item title
        // Log error if needed
        return false;
    }

    return true;
}

bool TIPDBShell::OpenItem(TIPInfo *item)
{
    QSqlQuery dbq(*db);
    bool ret = false;

    vector<IPTerm *> t;
    vector<IPShingle *> s;

    dbq.exec(QString("select id, term, weight, occurences, times from ip_term where ip_id = %1")
                 .arg(item->id));
    if (dbq.first()) {
        do {
            IPTerm *ti = new IPTerm(
                dbq.value(0).toInt(),
                dbq.value(1).toString(),
                dbq.value(2).toDouble(),
                dbq.value(4).toInt());
            t.push_back(ti);
        } while (dbq.next());
    }

    dbq.exec(QString("select id, term, weight, occurences, times, size, shows from "
                     "ip_term_shingles where ip_id = %1")
                 .arg(item->id));
    if (dbq.first()) {
        do {
            IPShingle *si = new IPShingle(
                dbq.value(0).toInt(),
                dbq.value(1).toString(),
                dbq.value(2).toDouble(),
                dbq.value(4).toInt());
            s.push_back(si);
        } while (dbq.next());
    }

    QString owner;
    owner.setNum(item->id);
    w = new ViewIP();
    w->setData(owner, item->ownerName, item->creationTime.toString(), item->name, t, s);
    w->show();

    return true;
}

double TIPDBShell::convertCommaSeparatedStringToDouble(const std::string &str)
{
    std::string modifiedStr = str;
    size_t pos = modifiedStr.find('.');
    if (pos != std::string::npos) {
        modifiedStr[pos] = ',';
    }
    return std::stod(modifiedStr);
}

bool TIPDBShell::loadFromFile(QString filename, TIPInfo *info, TIPFullInfo *full)
{
    tinyxml2::XMLDocument *doc = new tinyxml2::XMLDocument();
    tinyxml2::XMLError err = doc->LoadFile(filename.toLocal8Bit().data());
    if (err != tinyxml2::XML_SUCCESS)
        return false; // Unable to find or load file

    const tinyxml2::XMLElement *element = doc->FirstChildElement("portrait");
    if (!element)
        return false; // Unable to find tag <portrait>

    full->id = info->id = element->IntAttribute("id", 0);
    info->ownerName = full->creator_id
        = element->IntAttribute("creator_id", 0); // заменить на username
    full->comment = info->name = element->Attribute("comment");
    full->date_add = info->creationTime
        = QDateTime::fromString(element->Attribute("date_add"), "yyyy-MM-dd hh:mm:ss");
    info->sizeInTerms = element->IntAttribute("words", 0) + element->IntAttribute("shingles", 0);
    info->sizeInBytes = 0;
    info->compData1 = 0;
    info->compData2 = 0;
    info->compData3 = 0;

    full->object_id = element->IntAttribute("object_id", 0);
    full->aspect_id = element->IntAttribute("aspect_id", 0);
    full->attribute_id = element->IntAttribute("attribute_id", 0);
    full->norm_id = element->IntAttribute("norm_id", 0);

    full->ids_list = element->Attribute("ids_list");
    full->req_from = QDateTime::fromString(element->Attribute("req_from"), "yyyy-MM-dd hh:mm:ss");
    full->req_till = QDateTime::fromString(element->Attribute("req_till"), "yyyy-MM-dd hh:mm:ss");
    full->source_text = element->Attribute("source_text");
    full->words = element->IntAttribute("words", 0);
    full->abbreviations = element->Attribute("abbreviations");
    full->shingle_count = element->IntAttribute("shingles", 0);

    const tinyxml2::XMLElement *termtag = element->FirstChildElement("term");
    while (termtag) {
        TIPFullTermInfo *t = new TIPFullTermInfo;
        t->id = termtag->IntAttribute("id", 0);
        t->term = termtag->Attribute("term");
        std::string weightStr = termtag->Attribute("weight");
        t->weight = convertCommaSeparatedStringToDouble(weightStr);
        std::string occurencesStr = termtag->Attribute("occurences");
        t->occurences = convertCommaSeparatedStringToDouble(occurencesStr);
        t->times = termtag->IntAttribute("times", 0);

        full->terms.push_back(t);
        termtag = termtag->NextSiblingElement("term");
    }

    termtag = element->FirstChildElement("shingle");
    while (termtag) {
        TIPFullTermInfo *t = new TIPFullTermInfo;
        t->id = termtag->IntAttribute("id", 0);
        t->term = termtag->Attribute("term");
        t->weight = termtag->DoubleAttribute("weight", 0);
        t->occurences = termtag->DoubleAttribute("occurences", 0);
        t->times = termtag->IntAttribute("times", 0);
        t->size = termtag->IntAttribute("size", 0);
        t->shows = termtag->IntAttribute("shows", 0);

        full->shingles.push_back(t);
        termtag = termtag->NextSiblingElement("shingle");
    }

    return true;
}

bool TIPDBShell::saveToDB(TIPFullInfo *full)
{
    QSqlQuery dbq(*db);
    bool ret = false;

    if (full->id == 0) {
        dbq.exec("select nextval(\'ip_portret_id_seq\')");
        if (!dbq.first() || !dbq.seek(0, false)) {
            QSqlError res = db->lastError();
            // Unable to get next folder id
            // LogPrintf("? ERROR(%d): %s", res.number(), res.text().toStdString().c_str());
            // LogPrintf("? ..... at query: %s", query.toStdString().c_str());
            return false;
        }

        full->id = dbq.value(0).toInt();
    } else {
        ret = dbq.exec(QString("delete from ip_portrait where id = %1").arg(full->id));
        if (!ret) {
            QSqlError res = db->lastError();
            // Error deleting portrait
            // LogPrintf("? ERROR(%d): %s", res.number(), res.text().toStdString().c_str());
            // LogPrintf("? ..... at query: %s", query.toStdString().c_str());
            return false;
        }

        ret = dbq.exec(QString("delete from ip_term where ip_id = %1").arg(full->id));
        if (!ret) {
            QSqlError res = db->lastError();
            // Error deleting terms
            // LogPrintf("? ERROR(%d): %s", res.number(), res.text().toStdString().c_str());
            // LogPrintf("? ..... at query: %s", query.toStdString().c_str());
            return false;
        }

        ret = dbq.exec(QString("delete from ip_term_shingles where ip_id = %1").arg(full->id));
        if (!ret) {
            QSqlError res = db->lastError();
            // Error deleting portrait shingles
            // LogPrintf("? ERROR(%d): %s", res.number(), res.text().toStdString().c_str());
            // LogPrintf("? ..... at query: %s", query.toStdString().c_str());
            return false;
        }
    }

    QString sDateAdd
        = (full->date_add.isNull() || !full->date_add.isValid())
              ? QString("null")
              : QString("TIMESTAMP \'%1\'").arg(full->date_add.toString("yyyy-MM-dd hh:mm:ss"));
    QString sReqFrom
        = (full->req_from.isNull() || !full->req_from.isValid())
              ? QString("null")
              : QString("TIMESTAMP \'%1\'").arg(full->req_from.toString("yyyy-MM-dd hh:mm:ss"));
    QString sReqTill
        = (full->req_till.isNull() || !full->req_till.isValid())
              ? QString("null")
              : QString("TIMESTAMP \'%1\'").arg(full->req_till.toString("yyyy-MM-dd hh:mm:ss"));
    QString q
        = QString(
              "insert into ip_portrait (id, object_id, aspect_id, attribute_id, "
              "norm_id, creator_id, comment, ids_list, date_add, req_from, req_till, "
              "source_text, words, abbreviations, shingles) values "
              "(%1, %2, %3, %4, %5, %6, \'%7\', \'%8\', %9, %10, %11, \'%12\', %13, \'%14\', %15)")
              .arg(full->id)
              .arg(full->object_id)
              .arg(full->aspect_id)
              .arg(full->attribute_id)
              .arg(full->norm_id)
              .arg(full->creator_id)
              .arg(full->comment)
              .arg(full->ids_list)
              .arg(sDateAdd)
              .arg(sReqFrom)
              .arg(sReqTill)
              .
          // arg(full->source_text).arg(full->words).arg(full->abbreviations).arg(full->shingle_count)
          arg("")
              .arg(full->words)
              .arg(full->abbreviations)
              .arg(full->shingle_count);
    ret = dbq.exec(q);
    if (!ret) {
        QMessageBox::information(0, "Error running query", q);
        QSqlError res = db->lastError();
        // Error deleting portrait shingles
        // LogPrintf("? ERROR(%d): %s", res.number(), res.text().toStdString().c_str());
        // LogPrintf("? ..... at query: %s", query.toStdString().c_str());
        return false;
    }

    for (size_t i = 0; i < full->terms.size(); i++) {
        q = QString("insert into ip_term (id, ip_id, term, weight, occurences, times) values "
                    "(nextval(\'ipterm_id_seq\'), %1, "
                    "'%2', %3, %4, %5)")
                .arg(full->id)
                .arg(full->terms[i]->term)
                .arg(full->terms[i]->weight)
                .arg(full->terms[i]->occurences)
                .arg(full->terms[i]->times);
        ret = dbq.exec(q);

        if (!ret) {
            QMessageBox::information(0, "Error running query", q);
            QSqlError res = db->lastError();
            // Error deleting portrait shingles
            // LogPrintf("? ERROR(%d): %s", res.number(), res.text().toStdString().c_str());
            // LogPrintf("? ..... at query: %s", query.toStdString().c_str());
            return false;
        }
    }

    for (size_t i = 0; i < full->shingles.size(); i++) {
        dbq.prepare("INSERT INTO ip_term_shingles (id, ip_id, term, weight, occurences, times, "
                    "size, shows) "
                    "VALUES (nextval('ipterm_id_seq'), :ip_id, :term, :weight, :occurences, "
                    ":times, :size, :shows)");

        dbq.bindValue(":ip_id", QVariant::fromValue(full->id));
        dbq.bindValue(":term", QVariant::fromValue(full->shingles[i]->term));
        dbq.bindValue(":weight", QVariant::fromValue(full->shingles[i]->weight));
        dbq.bindValue(":occurences", QVariant::fromValue(full->shingles[i]->occurences));
        dbq.bindValue(":times", QVariant::fromValue(full->shingles[i]->times));
        dbq.bindValue(":size", QVariant::fromValue(full->shingles[i]->size));
        dbq.bindValue(":shows", QVariant::fromValue(full->shingles[i]->shows));

        if (!dbq.exec()) {
            QMessageBox::information(0, "Error running query", dbq.lastQuery());
            QSqlError res = dbq.lastError();
            // Error deleting portrait shingles
            // LogPrintf("? ERROR(%d): %s", res.number(), res.text().toStdString().c_str());
            // LogPrintf("? ..... at query: %s", query.toStdString().c_str());
            return false;
        }
    }

    return true;
}

TIPFullInfo *TIPDBShell::loadFromDB(int id)
{
    if (!id)
        return nullptr;

    QSqlQuery dbq(*db);
    dbq.exec(
        QString(
            "select object_id, aspect_id, attribute_id, norm_id, creator_id, comment, ids_list, "
            "date_add, req_from, req_till, source_text, words, abbreviations, shingles from "
            "ip_portrait where "
            "id = %1")
            .arg(id));
    if (!dbq.first() || !dbq.seek(0, false)) {
        QSqlError res = db->lastError();
        // Unable to get portrait parameters
        // LogPrintf("? ERROR(%d): %s", res.number(), res.text().toStdString().c_str());
        // LogPrintf("? ..... at query: %s", query.toStdString().c_str());
        return nullptr;
    }

    TIPFullInfo *full = new TIPFullInfo;
    full->id = id;
    full->object_id = dbq.value(0).toInt();
    full->aspect_id = dbq.value(1).toInt();
    full->attribute_id = dbq.value(2).toInt();
    full->norm_id = dbq.value(3).toInt();
    full->creator_id = dbq.value(4).toInt();
    full->comment = dbq.value(5).toString();
    full->ids_list = dbq.value(6).toString();
    full->date_add = dbq.value(7).toDateTime();
    full->req_from = dbq.value(8).toDateTime();
    full->req_till = dbq.value(9).toDateTime();
    full->source_text = dbq.value(10).toString();
    full->words = dbq.value(11).toInt();
    full->abbreviations = dbq.value(12).toString();
    full->shingle_count = dbq.value(13).toInt();

    dbq.exec(
        QString("select id, term, weight, occurences, times from ip_term where ip_id = %1").arg(id));
    if (dbq.first()) {
        do {
            TIPFullTermInfo *t = new TIPFullTermInfo;
            t->id = dbq.value(0).toInt();
            t->term = dbq.value(1).toString();
            t->weight = dbq.value(2).toDouble();
            t->occurences = dbq.value(3).toDouble();
            t->times = dbq.value(4).toInt();
            full->terms.push_back(t);
        } while (dbq.next());
    }

    dbq.exec(QString("select id, term, weight, occurences, times, size, shows from "
                     "ip_term_shingles where ip_id = %1")
                 .arg(id));
    if (dbq.first()) {
        do {
            TIPFullTermInfo *t = new TIPFullTermInfo;
            t->id = dbq.value(0).toInt();
            t->term = dbq.value(1).toString();
            t->weight = dbq.value(2).toDouble();
            t->occurences = dbq.value(3).toDouble();
            t->times = dbq.value(4).toInt();
            t->size = dbq.value(5).toInt();
            t->shows = dbq.value(6).toInt();
            full->shingles.push_back(t);
        } while (dbq.next());
    }

    return full;
}

bool TIPDBShell::saveToFile(TIPFullInfo *full, QString filename)
{
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLElement *root = doc.NewElement("portrait");
    doc.InsertFirstChild(root);

    root->SetAttribute("id", (int) full->id);
    root->SetAttribute("creator_id", (int) full->creator_id);
    root->SetAttribute("comment", full->comment.toStdString().c_str());
    root->SetAttribute(
        "date_add", full->date_add.toString("yyyy-MM-dd hh:mm:ss").toStdString().c_str());
    root->SetAttribute("object_id", (int) full->object_id);
    root->SetAttribute("aspect_id", (int) full->aspect_id);
    root->SetAttribute("attribute_id", (int) full->attribute_id);
    root->SetAttribute("norm_id", (int) full->norm_id);
    root->SetAttribute("ids_list", full->ids_list.toStdString().c_str());
    root->SetAttribute(
        "req_from", full->req_from.toString("yyyy-MM-dd hh:mm:ss").toStdString().c_str());
    root->SetAttribute(
        "req_till", full->req_till.toString("yyyy-MM-dd hh:mm:ss").toStdString().c_str());
    root->SetAttribute("source_text", full->source_text.toStdString().c_str());
    root->SetAttribute("words", (int) full->words);
    root->SetAttribute("abbreviations", full->abbreviations.toStdString().c_str());
    root->SetAttribute("shingles", (int) full->shingle_count);

    for (size_t i = 0; i < full->terms.size(); i++) {
        tinyxml2::XMLElement *element = doc.NewElement("term");
        element->SetAttribute("id", (int) full->terms[i]->id);
        element->SetAttribute("term", full->terms[i]->term.toStdString().c_str());
        element->SetAttribute("weight", full->terms[i]->weight);
        element->SetAttribute("occurences", full->terms[i]->occurences);
        element->SetAttribute("times", (int) full->terms[i]->times);
        root->InsertEndChild(element);
    }

    for (size_t i = 0; i < full->shingles.size(); i++) {
        tinyxml2::XMLElement *element = doc.NewElement("shingle");
        element->SetAttribute("id", (int) full->shingles[i]->id);
        element->SetAttribute("term", full->shingles[i]->term.toStdString().c_str());
        element->SetAttribute("weight", full->shingles[i]->weight);
        element->SetAttribute("occurences", full->shingles[i]->occurences);
        element->SetAttribute("times", (int) full->shingles[i]->times);
        element->SetAttribute("size", (int) full->shingles[i]->size);
        element->SetAttribute("shows", (int) full->shingles[i]->shows);
        root->InsertEndChild(element);
    }

    tinyxml2::XMLError err = doc.SaveFile(filename.toLocal8Bit().data());

    return (err == tinyxml2::XML_SUCCESS);
}

TIPInfo *TIPDBShell::CopyFileToDB(QString filename, folderid destination)
{
    TIPInfo *info = new TIPInfo;
    TIPFullInfo *full;

    if (destination < 1)
        destination = 1;

    QSqlQuery dbq(*db);

    if (filename.right(4).toUpper() == ".TXT") {
        QFile f(filename);
        QString str;
        if (f.open(QIODevice::ReadOnly))
            str = f.read(100000000);
        else {
            delete info;
            return nullptr;
        }

        IPPortrait *ip = new IPPortrait;
        QStringList sln;
        ip->GenerateByText(str, true, 0, 0, 0, &sln);
        full = new TIPFullInfo(ip);

        full->id = 0; // сохранять будем под новым ID
        full->comment = filename.section(QRegExp("[\\\\/]"), -1, -1, QString::SectionSkipEmpty);
        full->date_add = QDateTime::currentDateTime();
        full->creator_id = 1;
        bool res = saveToDB(full);

        // вычисление поля occurences
        res = dbq.exec(QString("update ip_term set occurences = times*1./(select sum(times) from "
                               "ip_term where ip_id = %1) "
                               "where ip_id = %1")
                           .arg(full->id));
        if (!res) {
            QSqlError res = db->lastError();
            // Unable to get next folder id
            // LogPrintf("? ERROR(%d): %s", res.number(), res.text().toStdString().c_str());
            // LogPrintf("? ..... at query: %s", query.toStdString().c_str());
            return nullptr;
        }

        // вычисление поля weight
        res = dbq.exec(QString("update ip_term b set weight = times*1./(select sum(c.times) from "
                               "ip_term c where c.ip_id = "
                               "%1) - (select sum(a.times) from ip_term a where a.term = "
                               "b.term)*1./(select sum(d.times) "
                               "from ip_term d) where b.ip_id = %1")
                           .arg(full->id));
        if (!res) {
            QSqlError res = db->lastError();
            // Unable to get next folder id
            // LogPrintf("? ERROR(%d): %s", res.number(), res.text().toStdString().c_str());
            // LogPrintf("? ..... at query: %s", query.toStdString().c_str());
            return nullptr;
        }
    } else if (filename.right(4).toUpper() == ".XML") {
        full = new TIPFullInfo;

        bool res = loadFromFile(filename, info, full);
        if (!res) {
            delete full;
            return nullptr;
        }

        full->id = 0; // сохранять будем под новым ID
        res = saveToDB(full);
    } else {
        delete info;
        return nullptr;
    }

    // добавление ip в папку
    bool res = dbq.exec(QString("insert into in_doc_collections (doc_id, coll_id) values (%1, %2)")
                            .arg(full->id)
                            .arg(destination));
    if (!res) {
        QSqlError res = db->lastError();
        // Unable to get next folder id
        // LogPrintf("? ERROR(%d): %s", res.number(), res.text().toStdString().c_str());
        // LogPrintf("? ..... at query: %s", query.toStdString().c_str());
        return nullptr;
    }

    return info;
}

bool TIPDBShell::CopyItemToFile(TIPInfo *item, QString filename)
{
    if (!item)
        return false;

    TIPFullInfo *p = loadFromDB(item->id);
    if (!p)
        return false; // Unable to load portrait from DB

    bool ret = saveToFile(p, filename);
    if (!ret)
        return false; // Unable to save file

    return true;
}

TIPFullInfo::TIPFullInfo(IPPortrait *ip)
{
    id = ip->ulId;
    object_id = ip->eObjectType;
    aspect_id = ip->eAspect;
    attribute_id = ip->eAttribute;
    norm_id = ip->eParamStruct;
    creator_id = ip->ulCreatorId;
    comment = ip->sComment;
    QStringList sl;
    for (size_t i = 0; i < ip->vObjectIds.size(); i++)
        sl.append(QString("%1").arg(ip->vObjectIds[i]));
    ids_list = sl.join(",");
    date_add = ip->dtCreateTime;
    req_from = ip->dtReqAfter;
    req_till = ip->dtReqBefore;
    source_text = ip->slSourceText.join("\n");
    words = ip->mData.size();
    abbreviations = ip->slAbbrs.join("\n");
    shingle_count = ip->mSData.size();

    for (auto it = ip->mData.begin(); it != ip->mData.end(); it++)
        terms.push_back(new TIPFullTermInfo(
            it->second.id,
            it->second.term,
            it->second.weight,
            it->second.occurences,
            it->second.times,
            0,
            0));

    for (auto it = ip->mSData.begin(); it != ip->mSData.end(); it++)
        shingles.push_back(new TIPFullTermInfo(
            it->second.id,
            it->second.term,
            it->second.weight,
            it->second.occurences,
            it->second.times,
            it->second.size,
            it->second.shows));
}
