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
    void indexFiles(
        const QString &dbName, bool calcWeightSim, const QMap<QString, DocumentData> &documents);

signals:
    void tokenReceived(bool success);
    void xmlFileDownloaded(bool success);

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

    QByteArray readFileToByteArray(const QString &filePath);
    QJsonDocument mapToJsonDocument(const QMap<QString, DocumentData> &documentMap);
};

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
    void tokenReceivedSignal(bool success);

public slots:
    void getXmlFile(const QString &filePath, long id, const QString &dbName)
    {
        emit getXmlFileSignal(filePath, id, dbName);
    }
    void onWorkerGotXmlFile(bool success);
};

#endif // SERVICEHANDLER_H
