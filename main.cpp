#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    setlocale(LC_ALL, "Russian");
//    QTextCodec* codec = QTextCodec::codecForName("UTF-8");
    QTextCodec* codec = QTextCodec::codecForName("Windows-1251");
//    QTextCodec::setCodecForTr(codec);
//    QTextCodec::setCodecForCStrings(codec);
    QTextCodec::setCodecForLocale(codec);

    MainWindow w;
    w.show();

    return a.exec();
}
