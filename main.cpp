#include "Components/magickonfug.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MagicKonfug w;
    w.show();

    return a.exec();
}
