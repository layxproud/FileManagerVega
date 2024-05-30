#ifndef SERVICEHANDLER_H
#define SERVICEHANDLER_H

#include "tipdbshell.h"
#include "trmlshell.h"
#include <curl/curl.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QObject>
#include <QThread>

/* КЛАСС ОБРАБОТКИ КУРЛОВ */

class NetworkWorker : public QObject
{
    Q_OBJECT

    std::string accessToken;

public:
    explicit NetworkWorker(QObject *parent = nullptr);
    ~NetworkWorker();

public slots:
    void getAccessToken(const QString &login, const QString &pass);
    void getXmlFile(const QString &filePath, long id, const QString &dbName);
    void deleteDbEntry(long id, const QString &dbName);
    void getSummary(const QString &filePath, long id, const QString &dbName);
    void indexFiles(
        const QString &dbName, bool calcWeightSim, const QMap<QString, DocumentData> &documents);
    void classifyPortraits(const QList<long> &portraitIDs, const QMap<QString, long> &classes);
    void clusterizePortraits(const QList<long> &portraitIDs, int clustersNum);
    void findMatchLevel(const FindMatchLevelParams &params);
    void findMatchingPortraits(const FindMatchParams &params);

signals:
    void tokenReceived(bool success);
    void xmlFileDownloaded(bool success);
    void clusterizationComplete(bool success, const QString &res);
    void classificationComplete(bool success, const QString &res);

private:
    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
    {
        ((std::string *) userp)->append((char *) contents, size * nmemb);
        return size * nmemb;
    }

    static size_t WriteData(void *ptr, size_t size, size_t nmemb, FILE *stream)
    {
        size_t written = fwrite(ptr, size, nmemb, stream);
        return written;
    }

    static size_t handle_header(void *ptr, size_t size, size_t nmemb, void *userdata)
    {
        std::string header((char *) ptr, size * nmemb);
        if (header.find("HTTP/") == 0) {
            int responseCode = std::stoi(header.substr(9, 3));
            *((int *) userdata) = responseCode;
        }
        return size * nmemb;
    }

    static QString getMimeType(const QString &filePath)
    {
        QFileInfo fileInfo(filePath);
        QString extension = fileInfo.suffix().toLower();

        static QMap<QString, QString> mimeTypes = {
            {"txt", "text/plain"},
            {"pdf", "application/pdf"},
            {"docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
            // Add more file types as needed
        };

        return mimeTypes.value(
            extension, "application/octet-stream"); // Default to binary stream if type is unknown
    }

    QByteArray readFileToByteArray(const QString &filePath);
    QJsonDocument mapToJsonDocument(const QMap<QString, DocumentData> &documentMap);
};

/* ИНТЕРФЕЙС ДЛЯ СВЯЗИ WORKSPACE С КУРЛАМИ */

class ServiceHandler : public QObject
{
    Q_OBJECT

    IPCompareResults comparisonResults;
    std::string accessToken;

public:
    explicit ServiceHandler(QObject *parent = nullptr);
    ~ServiceHandler();

    IPCompareResults comparePortraits(
        const std::vector<TIPFullTermInfo *> &leftTerms,
        const std::vector<TIPFullTermInfo *> &rightTerms);

private:
    NetworkWorker *worker;
    QThread *workerThread;

    void calculateComparisonParameters(
        const std::vector<TIPFullTermInfo *> &leftTerms,
        const std::vector<TIPFullTermInfo *> &rightTerms);
    void clearComparisonResults();
    void calculateComparisonCircles();

signals:
    void indexFilesSignal(
        const QString &dbName, bool calcWeightSim, const QMap<QString, DocumentData> &documents);
    void getAccessTokenSignal(const QString &login, const QString &pass);
    void getXmlFileSignal(const QString &filePath, long id, const QString &dbName);
    void getSummarySignal(const QString &filePath, long id, const QString &dbName);
    void tokenReceivedSignal(bool success);
    void deleteDbEntrySignal(long id, const QString &dbName);
    void classifyPortraitsSignal(const QList<long> &portraitIDs, const QMap<QString, long> &classes);
    void clusterizePortraitsSignal(const QList<long> &portraitIDs, int clustersNum);
    void findMatchLevelSignal(const FindMatchLevelParams &params);
    void findMatchingPortraitsSignal(const FindMatchParams &params);
    void clusterizationCompleteSignal(bool success, const QString &res);
    void classificationCompleteSignal(bool success, const QString &res);

public slots:
    void getXmlFile(const QString &filePath, long id, const QString &dbName)
    {
        emit getXmlFileSignal(filePath, id, dbName);
    }
    void getSummary(const QString &filePath, long id, const QString &dbName)
    {
        emit getSummarySignal(filePath, id, dbName);
    }
    void deleteDbEntry(long id, const QString &dbName) { emit deleteDbEntrySignal(id, dbName); }
    void onWorkerGotXmlFile(bool success);
};

#endif // SERVICEHANDLER_H
