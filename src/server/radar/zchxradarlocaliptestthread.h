#ifndef ZCHXRADARLOCALIPTESTTHREAD_H
#define ZCHXRADARLOCALIPTESTTHREAD_H

#include <QThread>

class zchxRadarLocalIPTestThread : public QThread
{
    Q_OBJECT
public:
    explicit zchxRadarLocalIPTestThread(const QString& host, int port, QObject *parent = 0);
    void    run();

signals:
    void signalSendLocalIP(const QString& ip);

public slots:
private:
    QString     mRemoteIP;
    int         mPort;
};

#endif // ZCHXRADARLOCALIPTESTTHREAD_H
