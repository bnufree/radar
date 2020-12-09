#ifndef TESTMAPWATCHDOGTHREAD_H
#define TESTMAPWATCHDOGTHREAD_H

#include <QThread>

class mapWatchDogThread : public QThread
{
    Q_OBJECT
public:
    explicit mapWatchDogThread(QObject *parent = 0);
    void run();

signals:

public slots:
};

#endif // TESTMAPWATCHDOGTHREAD_H
