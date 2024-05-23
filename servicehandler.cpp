#include "servicehandler.h"
#include <iostream>
#include <unordered_map>
#include <QDebug>

ServiceHandler::ServiceHandler(QObject *parent)
    : QObject{parent}
    , worker(new NetworkWorker())
    , workerThread(new QThread(this))
{
    worker->moveToThread(workerThread);
    connect(workerThread, &QThread::finished, worker, &QObject::deleteLater);

    connect(this, &ServiceHandler::indexFilesSignal, worker, &NetworkWorker::indexFiles);
    connect(this, &ServiceHandler::getAccessTokenSignal, worker, &NetworkWorker::getAccessToken);
    connect(this, &ServiceHandler::getXmlFileSignal, worker, &NetworkWorker::getXmlFile);

    connect(worker, &NetworkWorker::tokenReceived, this, &ServiceHandler::tokenReceivedSignal);
    connect(worker, &NetworkWorker::xmlFileDownloaded, this, &ServiceHandler::onWorkerGotXmlFile);
    workerThread->start();
}

ServiceHandler::~ServiceHandler()
{
    workerThread->quit();
    workerThread->wait();
    delete workerThread;
}

IPCompareResults ServiceHandler::comparePortraits(
    const std::vector<TIPFullTermInfo *> &leftTerms,
    const std::vector<TIPFullTermInfo *> &rightTerms)
{
    clearComparisonResults();
    calculateComparisonParameters(leftTerms, rightTerms);
    calculateComparisonCircles();
    return comparisonResults;
}

void ServiceHandler::calculateComparisonParameters(
    const std::vector<TIPFullTermInfo *> &leftTerms,
    const std::vector<TIPFullTermInfo *> &rightTerms)
{
    double c1, c2, d1, d2;
    std::unordered_map<QString, double> leftTermWeights;
    std::unordered_map<QString, double> rightTermWeights;

    for (const auto &term : leftTerms) {
        leftTermWeights[term->term] = term->weight;
    }

    for (const auto &term : rightTerms) {
        rightTermWeights[term->term] = term->weight;
    }

    c1 = 0;
    d1 = 0;
    for (const auto &term : leftTerms) {
        if (rightTermWeights.find(term->term) != rightTermWeights.end()) {
            c1 += term->weight;
        } else {
            d1 += term->weight;
        }
    }

    c2 = 0;
    d2 = 0;
    for (const auto &term : rightTerms) {
        if (leftTermWeights.find(term->term) != leftTermWeights.end()) {
            c2 += term->weight;
        } else {
            d2 += term->weight;
        }
    }
    comparisonResults.comm1 = c1;
    comparisonResults.comm2 = c2;
    comparisonResults.diff1 = d1;
    comparisonResults.diff2 = d2;
}

void ServiceHandler::clearComparisonResults()
{
    comparisonResults.comm1 = 0.0;
    comparisonResults.comm2 = 0.0;
    comparisonResults.diff1 = 0.0;
    comparisonResults.diff2 = 0.0;
    comparisonResults.r1 = 0.0;
    comparisonResults.r2 = 0.0;
    comparisonResults.d = 0.0;
}

void ServiceHandler::calculateComparisonCircles()
{
    double c1 = comparisonResults.comm1;
    double c2 = comparisonResults.comm2;
    double d1 = comparisonResults.diff1;
    double d2 = comparisonResults.diff2;
    double r1 = 0, r2 = 0, rmax = 100, rmin = 0, d = 0, nc2 = 0, nd2 = 0;
    nc2 = c1;
    nd2 = (c2 > 0.0000000001) ? d2 * c1 / c2 : d2;
    if (c1 + d1 > nc2 + nd2) {
        r1 = rmax;
        rmin = r2 = sqrt(100. * 100 * (nc2 + nd2) / (c1 + d1));
        if (c1 > 0)
            d = rmax + 2 * rmin * (0.5 - nc2 / (nc2 + nd2));
        else
            d = rmin + rmax;
    } else {
        r2 = rmax;
        rmin = r1 = sqrt(100. * 100 * (c1 + d1) / (nc2 + nd2));
        if (c2 > 0)
            d = rmax + 2 * rmin * (0.5 - c1 / (c1 + d1));
        else
            d = rmin + rmax;
    }
    comparisonResults.r1 = r1;
    comparisonResults.r2 = r2;
    comparisonResults.d = d;
}

void ServiceHandler::onWorkerGotXmlFile(bool success)
{
    if (success) {
        std::cout << "File downloaded successfully" << std::endl;
    } else {
        std::cerr << "Failed to download file" << std::endl;
    }
}

NetworkWorker::NetworkWorker(QObject *parent)
    : QObject{parent}
    , accessToken("")
{}

NetworkWorker::~NetworkWorker() {}

void NetworkWorker::getAccessToken(const QString &login, const QString &pass)
{
    CURL *curl;
    CURLcode res;
    std::string readBuffer;

    std::string url = "https://vega.mirea.ru/authservice.php?op=getusertoken&login="
                      + login.toStdString() + "&password=" + pass.toStdString();

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        } else {
            QByteArray jsonData = QByteArray::fromStdString(readBuffer);
            QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
            if (!jsonDoc.isNull()) {
                if (jsonDoc.isObject()) {
                    QJsonObject jsonObj = jsonDoc.object();
                    if (jsonObj.contains("token")) {
                        accessToken = jsonObj["token"].toString().toStdString();
                        emit tokenReceived(true);
                    } else {
                        std::cerr << "JSON parse error: 'token' not found" << std::endl;
                        emit tokenReceived(false);
                    }
                } else {
                    std::cerr << "JSON parse error: Document is not an object" << std::endl;
                    emit tokenReceived(false);
                }
            } else {
                std::cerr << "JSON parse error" << std::endl;
                emit tokenReceived(false);
            }
        }
        curl_easy_cleanup(curl);
    }
}

void NetworkWorker::getXmlFile(const QString &filePath, long id, const QString &dbName)
{
    CURL *curl;
    FILE *fp;
    CURLcode res;

    curl = curl_easy_init();
    if (!curl) {
        qDebug() << "Failed to initialize cURL";
        emit xmlFileDownloaded(false);
        return;
    }

    fp = fopen(filePath.toStdString().c_str(), "wb");
    if (!fp) {
        qDebug() << "Failed to open file " << filePath;
        emit xmlFileDownloaded(false);
        return;
    }

    std::string url = "https://vega.mirea.ru/intservice/index/xml/" + std::to_string(id);
    if (!dbName.isEmpty()) {
        url += "&db_name=" + dbName.toStdString();
    }

    qDebug() << QString::fromStdString(url);

    // Заголовок авторизации
    struct curl_slist *headers = nullptr;
    std::string authHeader = "Authorization: Bearer " + accessToken;
    headers = curl_slist_append(headers, authHeader.c_str());
    headers = curl_slist_append(headers, "accept: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        emit xmlFileDownloaded(false);
        return;
    }

    fclose(fp);
    curl_easy_cleanup(curl);

    emit xmlFileDownloaded(true);
    return;
}

void NetworkWorker::indexFiles(
    const QString &dbName, bool calcWeightSim, const QMap<QString, DocumentData> &documents)
{
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (!curl) {
        qDebug() << "Failed to initialize cURL";
        return;
    }

    QJsonDocument jsonDoc = mapToJsonDocument(documents);
    QByteArray jsonData = jsonDoc.toJson();

    curl_mime *mime = curl_mime_init(curl);

    // JSON
    curl_mimepart *jsonPart = curl_mime_addpart(mime);
    curl_mime_name(jsonPart, "json");
    curl_mime_data(jsonPart, jsonData.constData(), jsonData.size());
    curl_mime_type(jsonPart, "application/json");

    // Файлы
    for (auto it = documents.begin(); it != documents.end(); ++it) {
        const QString &filePath = it.key();
        QByteArray fileData = readFileToByteArray(filePath);

        if (fileData.isEmpty()) {
            continue;
        }

        curl_mimepart *filePart = curl_mime_addpart(mime);
        curl_mime_name(filePart, "files");
        curl_mime_data(filePart, fileData.constData(), fileData.size());
        QFileInfo fileInfo(filePath);
        curl_mime_filename(filePart, fileInfo.fileName().toStdString().c_str());
        curl_mime_type(filePart, "text/plain");
    }

    std::string url = "https://vega.mirea.ru/intservice/index/index_files?calc_weight_sim="
                      + std::string(calcWeightSim ? "true" : "false");
    if (!dbName.isEmpty()) {
        url += "&db_name=" + dbName.toStdString();
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    // Заголовок авторизации
    struct curl_slist *headers = nullptr;
    std::string authHeader = "Authorization: Bearer " + accessToken;
    headers = curl_slist_append(headers, authHeader.c_str());
    headers = curl_slist_append(headers, "accept: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        qDebug() << "cURL error: " << curl_easy_strerror(res);
    }

    curl_slist_free_all(headers);
    curl_mime_free(mime);
    curl_easy_cleanup(curl);

    qDebug() << "SUCCESS";
}

QByteArray NetworkWorker::readFileToByteArray(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file:" << filePath;
        return QByteArray();
    }
    return file.readAll();
}

QJsonDocument NetworkWorker::mapToJsonDocument(const QMap<QString, DocumentData> &documentMap)
{
    QJsonArray jsonArray;
    for (const auto &docData : documentMap) {
        jsonArray.append(docData.toJson());
    }

    QJsonObject mainObj;
    mainObj["documents"] = jsonArray;

    return QJsonDocument(mainObj);
}