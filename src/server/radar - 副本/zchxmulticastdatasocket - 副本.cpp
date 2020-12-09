#include "zchxmulticastdatasocket.h"
#include <QThread>
#include <QDateTime>
#include "zchxcommonUtils.h"
#include <QTimer>
#include <QNetworkInterface>

zchxMulticastDataScoket::zchxMulticastDataScoket(const QString& host,
                                                 int port,
                                                 QObject *parent)
    : QObject(parent),
      mHost(host),
      mPort(port),
      mSocket(Q_NULLPTR),
      mInit(false),
      mIsOk(false)
{
    mIfIndex = 0;
    mLastRecvTime.setMSecsSinceEpoch(0);
    mNetIFMap = zchxCommonUtils::getNetIfMap();
    mIfList = QNetworkInterface::allInterfaces();
    mLocalIPList = zchxCommonUtils::getAllIpv4List();
    QTimer* mSwapIPTimer = new QTimer(this);
    mSwapIPTimer->setInterval(2000);
    connect(mSwapIPTimer, SIGNAL(timeout()), this, SLOT(slotCheckDataRecv()));
    QTimer::singleShot(100, this, SLOT(init()));
//    qDebug()<<"current ipv4:"<<mLocalIPList;
    mSwapIPTimer->start();
}

zchxMulticastDataScoket::~zchxMulticastDataScoket()
{
    if(mSocket)
    {
        mSocket->abort();
        mSocket->close();
        mSocket->deleteLater();
    }

}

void zchxMulticastDataScoket::writeDatagram(const QByteArray &data)
{
    if(!mSocket) return ;
    mSocket->writeDatagram(data, QHostAddress(mHost), mPort);
}

void zchxMulticastDataScoket::init()
{
    mInit = false;
    mErrCunt = 0;
    if(mHost.length() == 0 || mPort == 0) return;
    qDebug()<<"current ip index:"<<mIfIndex;
    for(int i=mIfIndex; i<mLocalIPList.size(); i++)
    {
        QString ip_str = mLocalIPList[i];

        mIfIndex = (i+1)%mLocalIPList.size();
        if(mSocket)
        {
            mSocket->deleteLater();
            mSocket = 0;
        }
        mSocket = new QUdpSocket();
        //选择网卡
        if(!mSocket->bind(QHostAddress(ip_str), mPort,QAbstractSocket::ShareAddress))
        {
            qDebug()<<"bind port failed:"<<mPort<<" with local card:"<<ip_str;
            continue;
        }

        mSocket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, 0);//禁止本机接收
//        mSocket->setMulticastInterface(_if);
        if(!mSocket->joinMulticastGroup(QHostAddress(mHost)))
        {
            qDebug()<<"joinMuticastGroup host failed:"<<mHost<<" with local card:"<<ip_str;
            continue;
        }
        qDebug()<<"init multicast succeed."<<mHost<<":"<<mPort<<" with local card:"<<ip_str;
        //开始接受数据检测
        connect(mSocket, SIGNAL(readyRead()),this, SLOT(slotReadyReadMulticastData()));
        connect(mSocket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(slotDisplayUdpReportError(QAbstractSocket::SocketError)));
        mInit = true;

        break;
    }
    mIfIndex = mIfIndex % mLocalIPList.size();
    return;
}


void zchxMulticastDataScoket::slotReadyReadMulticastData()
{
//    if(!mInit || !mSocket) return;
    mLastRecvTime = QDateTime::currentDateTime();
    QByteArray datagram;
    // 让datagram的大小为等待处理的数据报的大小，这样才能接收到完整的数据
    qint64 dataSize = mSocket->pendingDatagramSize();
    datagram.resize(dataSize);//pendingDatagramSize() 当前数据包大小    
    qDebug()<<"recv data from socket:"<<dataSize<<mSocket->localAddress()<<mHost;
    // 接收数据报，将其存放到datagram中
    mSocket->readDatagram(datagram.data(), dataSize);
    emit signalSendRecvData(datagram);
}

bool zchxMulticastDataScoket::isFine() const
{
    if(isInit()) return mLastRecvTime.secsTo(QDateTime::currentDateTime()) < 60;
    return false;
}


void zchxMulticastDataScoket::slotDisplayUdpReportError(QAbstractSocket::SocketError e)
{
    qDebug()<<"socket error:"<<e<<mSocket->errorString()<<mHost;
}

void zchxMulticastDataScoket::slotCheckDataRecv()
{
    if(isFine())
    {
        mErrCunt = 0;
        QTimer* timer = qobject_cast<QTimer*>(sender());
        if(timer)timer->stop();
    } else
    {
        if(mSocket) qDebug()<<"get data from socket failed."<<mSocket->localAddress()<<mHost;
        mErrCunt++;
        if(mErrCunt == 10)
        {
            init();
        }
    }
}
