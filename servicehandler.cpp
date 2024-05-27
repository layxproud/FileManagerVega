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
    connect(this, &ServiceHandler::deleteDbEntrySignal, worker, &NetworkWorker::deleteDbEntry);
    connect(this, &ServiceHandler::getSummarySignal, worker, &NetworkWorker::getSummary);
    connect(this, &ServiceHandler::classifyPortraitsSignal, worker, &NetworkWorker::classifyPortraits);
    connect(
        this,
        &ServiceHandler::clusterizePortraitsSignal,
        worker,
        &NetworkWorker::clusterizePortraits);

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
        qDebug() << "Файл успешно загружен";
    } else {
        qCritical() << "Не удалось загрузить файл";
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

    curl = curl_easy_init();
    if (!curl) {
        qCritical() << "Не удалось инициализировать cURL";
        emit tokenReceived(false);
        return;
    }

    std::string url = "https://vega.mirea.ru/authservice.php?op=getusertoken&login="
                      + login.toStdString() + "&password=" + pass.toStdString();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    std::string readBuffer;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    int httpResponseCode = 0;
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, handle_header);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &httpResponseCode);

    auto cleanup = qScopeGuard([&] { curl_easy_cleanup(curl); });

    res = curl_easy_perform(curl);
    if (res != CURLE_OK || httpResponseCode != 200) {
        qCritical() << "Ошибка в curl_easy_perform() или код ответа не 200: "
                    << QString::fromStdString(curl_easy_strerror(res))
                    << "Код: " << QString::number(httpResponseCode);
        emit tokenReceived(false);
        return;
    }

    QByteArray jsonData = QByteArray::fromStdString(readBuffer);
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    if (jsonDoc.isNull() || !jsonDoc.isObject()) {
        qCritical() << "Ошибка чтения JSON ответа";
        emit tokenReceived(false);
        return;
    }
    QJsonObject jsonObj = jsonDoc.object();
    if (!jsonObj.contains("token")) {
        qCritical() << "Ошибка JSON: 'token' отсутствует";
        emit tokenReceived(false);
        return;
    }
    accessToken = jsonObj["token"].toString().toStdString();
    emit tokenReceived(true);
}

void NetworkWorker::getXmlFile(const QString &filePath, long id, const QString &dbName)
{
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (!curl) {
        qCritical() << "Не удалось инициализировать cURL";
        emit xmlFileDownloaded(false);
        return;
    }

    FILE *fp;
    fp = fopen(filePath.toStdString().c_str(), "wb");
    if (!fp) {
        qCritical() << "Не удалось открыть файл " << filePath;
        emit xmlFileDownloaded(false);
        curl_easy_cleanup(curl);
        return;
    }
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

    std::string url = "https://vega.mirea.ru/intservice/index/xml/" + std::to_string(id);
    if (!dbName.isEmpty()) {
        url += "?db_name=" + dbName.toStdString();
    }
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    struct curl_slist *headers = nullptr;
    std::string authHeader = "Authorization: Bearer " + accessToken;
    headers = curl_slist_append(headers, authHeader.c_str());
    headers = curl_slist_append(headers, "accept: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    int httpResponseCode = 0;
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, handle_header);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &httpResponseCode);

    auto cleanup = qScopeGuard([&] {
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    });

    res = curl_easy_perform(curl);
    if (res != CURLE_OK || httpResponseCode != 200) {
        qCritical() << "Ошибка в curl_easy_perform() или код ответа не 200: "
                    << QString::fromStdString(curl_easy_strerror(res))
                    << "Код: " << QString::number(httpResponseCode);
        emit xmlFileDownloaded(false);
        if (fp) {
            fclose(fp);
            remove(filePath.toStdString().c_str());
        }
        return;
    }

    fclose(fp);
    emit xmlFileDownloaded(true);
}

void NetworkWorker::deleteDbEntry(long id, const QString &dbName)
{
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (!curl) {
        qCritical() << "Не удалось инициализировать cURL";
        return;
    }

    std::string url = "https://vega.mirea.ru/intservice/index/delete/" + std::to_string(id);
    if (!dbName.isEmpty()) {
        url += "?db_name=" + dbName.toStdString();
    }
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    std::string readBuffer;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    struct curl_slist *headers = nullptr;
    std::string authHeader = "Authorization: Bearer " + accessToken;
    headers = curl_slist_append(headers, authHeader.c_str());
    headers = curl_slist_append(headers, "accept: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    int httpResponseCode = 0;
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, handle_header);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &httpResponseCode);

    auto cleanup = qScopeGuard([&] {
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    });

    res = curl_easy_perform(curl);
    if (res != CURLE_OK || httpResponseCode != 200) {
        qCritical() << "Ошибка в curl_easy_perform() или код ответа не 200: "
                    << QString::fromStdString(curl_easy_strerror(res))
                    << "Код: " << QString::number(httpResponseCode);
        return;
    }

    qDebug() << "Удаление завершено";
}

void NetworkWorker::getSummary(const QString &filePath, long id, const QString &dbName)
{
    qDebug() << "Получение реферата";
}

void NetworkWorker::indexFiles(
    const QString &dbName, bool calcWeightSim, const QMap<QString, DocumentData> &documents)
{
    if (documents.isEmpty()) {
        qCritical() << "Не передано ни одного файла";
        return;
    }

    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (!curl) {
        qCritical() << "Не удалось инициализировать cURL";
        return;
    }

    std::string url = "https://vega.mirea.ru/intservice/index/index_files?calc_weight_sim="
                      + std::string(calcWeightSim ? "true" : "false");
    if (!dbName.isEmpty()) {
        url += "&db_name=" + dbName.toStdString();
    }
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

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
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

    struct curl_slist *headers = nullptr;
    std::string authHeader = "Authorization: Bearer " + accessToken;
    headers = curl_slist_append(headers, authHeader.c_str());
    headers = curl_slist_append(headers, "accept: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    int httpResponseCode = 0;
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, handle_header);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &httpResponseCode);

    auto cleanup = qScopeGuard([&] {
        curl_slist_free_all(headers);
        curl_mime_free(mime);
        curl_easy_cleanup(curl);
    });

    res = curl_easy_perform(curl);
    if (res != CURLE_OK || httpResponseCode != 200) {
        qCritical() << "Ошибка в curl_easy_perform() или код ответа не 200: "
                    << QString::fromStdString(curl_easy_strerror(res))
                    << "Код: " << QString::number(httpResponseCode);
        return;
    }

    qDebug() << "Индексация прошла успешно";
}

void NetworkWorker::classifyPortraits(const QList<long> &portraitIDs, const QList<long> &classesIDs)
{
    qDebug() << "Классифкация портретов";
    qDebug() << portraitIDs;
    qDebug() << classesIDs;
}

void NetworkWorker::clusterizePortraits(const QList<long> &portraitIDs, int clustersNum)
{
    qDebug() << "Кластеризация портретов";
    qDebug() << portraitIDs;
    qDebug() << clustersNum;
}

QByteArray NetworkWorker::readFileToByteArray(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Не удалось открыть файл:" << filePath;
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
