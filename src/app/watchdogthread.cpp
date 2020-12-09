#include "watchdogthread.h"
#include <QDir>
#include <QApplication>

mapWatchDogThread::mapWatchDogThread(QObject *parent) : QThread(parent)
{

}

void mapWatchDogThread::run()
{
    QDir dir(QApplication::applicationDirPath() + QString("/watchdog"));
    if(!dir.exists())
    {
        dir.mkpath(dir.path());
    }
    QString fileName = QString("%1/%2.txt").arg(dir.path()).arg(QApplication::applicationName());
    QFile file(fileName);
    while (true) {
        if(file.open(QIODevice::WriteOnly))
        {
            file.write("ddddd");
            file.close();
        }
        sleep(30);
    }
}
