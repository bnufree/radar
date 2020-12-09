#include "zchxradarlocaliptestthread.h"
#include "zchxmulticastdatasocket.h"
#include "zchxcommonutils.h"
#include <QDebug>

zchxRadarLocalIPTestThread::zchxRadarLocalIPTestThread(const QString& host, int port, QObject *parent) : QThread(parent)
{
    mRemoteIP = host;
    mPort = port;
}

void zchxRadarLocalIPTestThread::run()
{
    QStringList iplist = zchxCommonUtils::getAllIpv4List();
    if(iplist.size() == 1)
    {
        emit signalSendLocalIP(iplist.first());
        return;
    }
    //开启检测
//    foreach (QString local_ip, iplist) {
//        zchxMulticastDataScoket *socket = new zchxMulticastDataScoket(mRemoteIP, mPort, local_ip, "Test", 1, true, 0, ModeSyncRecv);
//        if(!socket && !socket->isFine())
//        {
//            if(socket) delete socket;
//            qDebug()<<"error found when test local ip"<<local_ip;
//            continue;
//        }
//        socket->setIsTestLocalIP(true);
//        socket->startRecv();
//        if(socket->isDataRecved())
//        {
//            qDebug()<<"found local ip can receive udp data:"<<local_ip;
//            emit signalSendLocalIP(local_ip);
//            break;
//        }
//    }

    return;

}
