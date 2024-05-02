#include "xmlparser.h"
#include <QDebug>

XmlParser::XmlParser()
{

}

XmlParser::ParsedData XmlParser::readFileAndParse(const QString &filePath)
{
    ParsedData parsedData;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Could not open file for reading.";
        return parsedData;
    }

    QString xmlContent = file.readAll();
    file.close();

    parseXML(xmlContent, parsedData);

    return parsedData;
}

void XmlParser::parseXML(const QString &xmlContent, ParsedData &parsedData)
{
    QDomDocument doc;
    if (!doc.setContent(xmlContent)) {
        qDebug() << "Failed to parse XML";
        return;
    }

    QDomElement root = doc.documentElement();
    if (root.tagName() != "portrait") {
        qDebug() << "Root element is not 'portrait'";
        return;
    }

    // Параметры корневого элемента
    parsedData.id = root.attribute("id");
    parsedData.user = root.attribute("creator_id");
    parsedData.date = root.attribute("date_add");
    parsedData.comment = root.attribute("comment");

    // Термины
    QDomNodeList termNodes = root.elementsByTagName("term");
    for (int i = 0; i < termNodes.count(); ++i) {
        QDomElement termElement = termNodes.at(i).toElement();
        if (!termElement.isNull()) {
            QString termId = termElement.attribute("id");
            QString term = termElement.attribute("term");
            QString weightStr = termElement.attribute("weight");
            weightStr.replace(',', '.'); // Без замены не распознает
            double weight = weightStr.toDouble();
            int times = termElement.attribute("times").toInt();

            IPTerm *termObj = new IPTerm(termId.toInt(), term, weight, times);
            parsedData.terms.push_back(termObj);
        }
    }

    // Шинглы
    QDomNodeList shingleNodes = root.elementsByTagName("shingle");
    for (int i = 0; i < shingleNodes.count(); ++i) {
        QDomElement shingleElement = shingleNodes.at(i).toElement();
        if (!shingleElement.isNull()) {
            QString shingleId = shingleElement.attribute("id");
            QString shingle = shingleElement.attribute("term");
            QString weightStr = shingleElement.attribute("weight");
            weightStr.replace(',', '.'); // Без замены не распознает
            double weight = weightStr.toDouble();
            int times = shingleElement.attribute("times").toInt();

            IPShingle *shingleObj = new IPShingle(shingleId.toInt(), shingle, weight, times);
            parsedData.shingles.push_back(shingleObj);
        }
    }
}
