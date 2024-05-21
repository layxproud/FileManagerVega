#ifndef SERVICEHANDLER_H
#define SERVICEHANDLER_H

#include <QObject>
#include <curl/curl.h>
#include "trmlshell.h"
#include "tipdbshell.h"
#include <QJsonDocument>
#include <QJsonObject>

class ServiceHandler : public QObject
{
    Q_OBJECT

    IPCompareResults comparisonResults;
    std::string accessToken;

public:
    explicit ServiceHandler(QObject *parent = nullptr);
    ~ServiceHandler();

    void receiveAccessToken(const QString& login, const QString& pass);
    void setAccessToken(const std::string &token);
    IPCompareResults comparePortraits(const std::vector<TIPFullTermInfo *> &leftTerms,
                                      const std::vector<TIPFullTermInfo *> &rightTerms);
    bool getXMLFile(const std::string &url, const std::string &filePath);

private:
    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    static size_t WriteData(void *ptr, size_t size, size_t nmemb, FILE *stream) {
        size_t written = fwrite(ptr, size, nmemb, stream);
        return written;
    }

    void calculateComparisonParameters(const std::vector<TIPFullTermInfo *> &leftTerms,
                                       const std::vector<TIPFullTermInfo *> &rightTerms);
    void clearComparisonResults();
    void calculateComparisonCircles();

signals:
    void tokenReceived();
};

#endif // SERVICEHANDLER_H
