#include "zchxvalidationthread.h"
#include "zchxregistorchecker.h"

zchxValidationThread::zchxValidationThread(QObject *parent) : QThread(parent)
{
    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
}

void zchxValidationThread::run()
{
    while (true) {
        zchxRegistorChecker check;
        QString msg;
        if(!check.startCheck(msg))
        {
            check.removeKey();
            emit signalAppExpired();
            break;
        }

        sleep(10);
    }
}
