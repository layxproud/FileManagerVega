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
public:
    explicit NetworkWorker(QObject *parent = nullptr);
    ~NetworkWorker();

public slots:
    void getAccessToken(const QString &login, const QString &pass);
    void getXmlFile(const QString &url, const QString &filePath);
    void indexFiles(const QMap<QString, DocumentData> &documents);

signals:
    void tokenReceived(bool success);
    void xmlFileDownloaded(bool success);

private:
    std::string accessToken;
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
    void tokenReceived(bool success);

public slots:
    void getAccessToken(const QString &login, const QString &pass);
    void getXmlFile(const QString &url, const QString &filePath);
    void indexFiles(const QMap<QString, DocumentData> &documents);

    void onWorkerReceivedToken(bool success);
    void onWorkerGotXmlFile(bool success);
};

#endif // SERVICEHANDLER_H
