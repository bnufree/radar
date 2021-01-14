#include "zchxmulticastdatasocket.h"
#include <QThread>
#include <QDateTime>
#include <QTimer>
#include <QNetworkInterface>

zchxMulticastDataScoket::zchxMulticastDataScoket(const QString & ip, const QString& host,
                                                 int port,
                                                 bool data_recv_chk = true,
                                                 const QNetworkInterface nif,
                                                 QObject *parent)
    : QUdpSocket(parent),
      mHost(host),
      mPort(port),
      mInit(false),
      mIsOk(false),
      mLocalIP(ip),
      mNIF(nif),
      mDebug(false)
{
    mLastRecvTime.setMSecsSinceEpoch(0);
    qRegisterMetaType<QAbstractSocket::SocketState>("QAbstractSocket::SocketState");
    //开始接受数据检测
    connect(this, SIGNAL(readyRead()),this, SLOT(slotReadyReadMulticastData()));
    connect(this, SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(slotDisplayUdpReportError(QAbstractSocket::SocketError)));
    connect(this, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(slotStateChanged(QAbstractSocket::SocketState)));
    QTimer::singleShot(100, this, SLOT(init()));
    if(data_recv_chk)   QTimer::singleShot(1000 * 10, this, SLOT(slotCheckDataRecv()));
}

zchxMulticastDataScoket::~zchxMulticastDataScoket()
{
    abort();
    close();
    qDebug()<<"destruct :"<<mHost<<mLocalIP;
}

void zchxMulticastDataScoket::writeData(const QByteArray &data)
{
    int size = writeDatagram(data, QHostAddress(mHost), mPort);
//    qDebug()<<"write dato to host:"<<mHost<<mPort<<" with size:"<<size;
}

void zchxMulticastDataScoket::init()
{
    abort();
    mInit = false;
    mErrCunt = 0;
    if(mHost.length() == 0 || mPort == 0) return;
    QString bind_ip = mLocalIP;
#ifndef Q_OS_WIN
    bind_ip = mHost;
#endif
    if(!bind(QHostAddress(bind_ip), mPort,QAbstractSocket::ShareAddress))
    {
        qDebug()<<"bind port failed:"<<mPort<<" with local card:"<<bind_ip;
        emit signalNoDataRecv();
        return;
    }
    setSocketOption(QAbstractSocket::MulticastLoopbackOption, 0);//禁止本机接收
    setMulticastInterface(mNIF);
    if(!joinMulticastGroup(QHostAddress(mHost), mNIF))
    {
        qDebug()<<"joinMuticastGroup host failed:"<<mHost<<" with local card:"<<mLocalIP;
        emit signalNoDataRecv();
        return;
    }
    qDebug()<<"init multicast succeed."<<mHost<<":"<<mPort<<" with local card:"<<mLocalIP;
    mInit = true;
    return;
}


void zchxMulticastDataScoket::slotReadyReadMulticastData()
{
    while (hasPendingDatagrams()) {
        mLastRecvTime = QDateTime::currentDateTime();
        QByteArray datagram;
        // 让datagram的大小为等待处理的数据报的大小，这样才能接收到完整的数据
        qint64 dataSize = pendingDatagramSize();
        datagram.resize(dataSize);//pendingDatagramSize() 当前数据包大小
        if(mDebug)  qDebug()<<"recv data from socket:"<<dataSize<<localAddress()<<mHost<<QThread::currentThread();
        // 接收数据报，将其存放到datagram中
        readDatagram(datagram.data(), dataSize);
        emit signalSendRecvData(datagram);
    }
}

bool zchxMulticastDataScoket::isFine() const
{
    if(!mInit) return false;
    return mLastRecvTime.secsTo(QDateTime::currentDateTime()) < 10;
}


void zchxMulticastDataScoket::slotDisplayUdpReportError(QAbstractSocket::SocketError e)
{
//    qDebug()<<"socket error:"<<e<<errorString()<<localAddress()<<mHost;
}

void zchxMulticastDataScoket::slotStateChanged(QAbstractSocket::SocketState e)
{
//    qDebug()<<"socket state:"<<e<<localAddress()<<mHost;
}

void zchxMulticastDataScoket::slotCheckDataRecv()
{
    if(!isFine())
    {
        qDebug()<<"no data recv from"<<mHost<<":"<<mPort<<" with local card:"<<mLocalIP;
        emit signalNoDataRecv();
    }
}
