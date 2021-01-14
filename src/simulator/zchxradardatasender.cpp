#include "zchxradardatasender.h"
#include "zchxmulticastdatasocket.h"


extern QNetworkInterface findNIF(const QString& ip);
extern QStringList getAllIpv4List();


ZCHXRadarDataSender::ZCHXRadarDataSender(const QString& ip, int port,
                                         const QString& local_ip,
                                         QObject *parent)
    : QObject(parent),
      mLocalIP(local_ip),
      mUdpIP(ip),
      mUdpPort(port),
      mSocket(0)

{
    mLocalIPList = getAllIpv4List();
    mCurIPIndex = mLocalIP.indexOf(mLocalIP);
    if(mCurIPIndex < 0) mCurIPIndex = 0;
    init();


}

ZCHXRadarDataSender::~ZCHXRadarDataSender()
{
    if(mSocket)
    {
        delete mSocket;
        mSocket = NULL;
    }
}

void ZCHXRadarDataSender::init()
{
    if(mSocket)
    {
        delete mSocket;
        mSocket = NULL;
    }
    if(mCurIPIndex >= mLocalIPList.size()) mCurIPIndex = 0;

    mLocalIP = mLocalIPList[mCurIPIndex++];
    QNetworkInterface nif = findNIF(mLocalIP);
    mSocket = new zchxMulticastDataScoket(mLocalIP, mUdpIP, mUdpPort, false, nif);
    connect(mSocket, &zchxMulticastDataScoket::signalNoDataRecv, this, [=](){
        qDebug()<<"init socket failed";
        init();
    });
}

void ZCHXRadarDataSender::slotRecvSendData(const QByteArray &sRadarData, int size)
{
    if(mSocket && mSocket->isInit()) mSocket->writeData(sRadarData);
}


