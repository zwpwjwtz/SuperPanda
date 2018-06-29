#include "global.h"
#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    appTranslator.load(QString(":/Translations/SuperPanda_")
                       .append(QLocale::system().name()));
    a.installTranslator(&appTranslator);

    MainWindow w;
    w.show();

    return a.exec();
}
