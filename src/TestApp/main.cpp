#include "zchxmainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    zchxMainWindow w;
    w.show();

    return a.exec();
}
