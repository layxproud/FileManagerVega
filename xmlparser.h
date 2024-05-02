#ifndef XMLPARSER_H
#define XMLPARSER_H

#include <QDomDocument>
#include <QDomElement>
#include <QDomNodeList>
#include <QFile>
#include "trmlshell.h"

class XmlParser
{
public:
    struct ParsedData {
        QString id;
        QString user;
        QString date;
        QString comment;
        std::vector<IPTerm*> terms;
        std::vector<IPShingle*> shingles;
    };
    XmlParser();
    ParsedData readFileAndParse(const QString& filePath);

private:
    void parseXML(const QString& xmlContent, ParsedData &parsedData);
};

#endif // XMLPARSER_H
