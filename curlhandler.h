#ifndef CURLHANDLER_H
#define CURLHANDLER_H

#include <QObject>
#include <curl/curl.h>

class CurlHandler : public QObject
{
    Q_OBJECT
public:
    explicit CurlHandler(QObject *parent = nullptr);

signals:

};

#endif // CURLHANDLER_H
