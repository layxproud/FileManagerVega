#include "mainwindow.h"
#include "trmlshell.h"
#include <QApplication>
#include <QDebug>
#include <QFile>

void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);
    QString txt;
    switch (type) {
    case QtDebugMsg:
        txt = QString("Debug: %1").arg(msg);
        break;
    case QtWarningMsg:
        txt = QString("Warning: %1").arg(msg);
        break;
    case QtCriticalMsg:
        txt = QString("Critical: %1").arg(msg);
        break;
    case QtFatalMsg:
        txt = QString("Fatal: %1").arg(msg);
        break;
    }

    qDebug() << txt;
    QFile outputFile("debug.log");
    if (outputFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream outputStream(&outputFile);
        outputStream << txt << endl;
    } else {
        qDebug() << "Failed to open log file";
    }
}

int main(int argc, char *argv[])
{
    // Очистка файла
    QFile outputFile("debug.log");
    if (outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        outputFile.close();
    } else {
        qDebug() << "Failed to open log file for clearing";
    }
    // qputenv("QT_DEBUG_PLUGINS", QByteArray("1"));
    qInstallMessageHandler(customMessageHandler);

    QApplication a(argc, argv);
    setlocale(LC_ALL, "Russian");
    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QTextCodec::setCodecForLocale(codec);

    qRegisterMetaType<QMap<QString, DocumentData>>("QMap<QString,DocumentData>");
    qRegisterMetaType<QList<long>>("QList<long>");
    qRegisterMetaType<QMap<QString, long>>("QMap<QString, long>");
    qRegisterMetaType<QMap<long, QString>>("QMap<long, QString>");
    qRegisterMetaType<FindMatchParams>("FindMatchParams");
    qRegisterMetaType<FindMatchLevelParams>("FindMatchLevelParams");

    MainWindow w;
    w.show();

    return a.exec();
}
