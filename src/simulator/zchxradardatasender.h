#ifndef ZCHXRADARDATASENDER_H
#define ZCHXRADARDATASENDER_H

#include <QObject>
#include <QTimer>
#include <QThread>
#include "radarccontroldefines.h"
#include <QNetworkInterface>


class zchxMulticastDataScoket;


class ZCHXRadarDataSender : public QObject
{
    Q_OBJECT
public:
    explicit ZCHXRadarDataSender(const QString& ip, int port, const QString& local_ip = QString(),
                                 QObject *parent = 0);
    ~ZCHXRadarDataSender();
private:
    void init();

public slots:
    void slotRecvSendData(const QByteArray &sRadarData, int);

private:

    zchxMulticastDataScoket*            mSocket;
    QStringList                         mLocalIPList;
    QString                             mLocalIP;
    int                                 mCurIPIndex;
    QString                             mUdpIP;
    int                                 mUdpPort;
};

#endif // ZCHXRADARDATASENDER_H
