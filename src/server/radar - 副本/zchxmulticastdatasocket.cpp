#include "zchxmulticastdatasocket.h"
#include <QThread>
#include <QDateTime>
#include "zchxcommonUtils.h"
#include <QTimer>
#include <QNetworkInterface>

#ifndef UDP_THREAD

zchxMulticastDataScoket::zchxMulticastDataScoket(const QString & ip, const QString& host,
                                                 int port,
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
    QTimer::singleShot(1000 * 10, this, SLOT(slotCheckDataRecv()));
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
    //选择网卡
    if(!bind(QHostAddress(mLocalIP), mPort,QAbstractSocket::ShareAddress))
    {
        qDebug()<<"bind port failed:"<<mPort<<" with local card:"<<mLocalIP;
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
//    if(waitForReadyRead(3000))
//    {
//        qDebug()<<"after 3000ms no data recv from"<<mHost<<":"<<mPort<<" with local card:"<<mLocalIP;
//        emit signalNoDataRecv();
//    }
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
//        emit signalSendRecvData(datagram);
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

#else

zchxMulticastDataScoket::zchxMulticastDataScoket(const QString & ip, const QString& host,
                                                 int port,
                                                 const QNetworkInterface nif,
                                                 QObject *parent)
    : QThread(parent),
      mHost(host),
      mPort(port),
      mLocalIP(ip),
      mNIF(nif),
      mDebug(false),
      mIsOver(false)
{
    mLastRecvTime.setMSecsSinceEpoch(0);
    QTimer::singleShot(100, this, [=](){
        start();
    });
}

zchxMulticastDataScoket::~zchxMulticastDataScoket()
{
    qDebug()<<"destruct :"<<mHost<<mLocalIP<<" start";
    setIsOver(true);
    terminate();
    bool sts = wait(3000);
    qDebug()<<"destruct :"<<mHost<<mLocalIP<<" end"<<sts;
}

void zchxMulticastDataScoket::writeData(const QByteArray &data)
{
    QMutexLocker locker(&mMutex);
    mSendDataList.append(data);
}

void zchxMulticastDataScoket::run()
{
    qDebug()<<"start socket thread........";
    QUdpSocket *socket = 0;
    int error_num = 0;
    while (!mIsOver) {
        if(!socket)
        {
            socket = init();
            if(!socket) break;
        }
        //发送数据
        if(mSendDataList.size() > 0)
        {
            QMutexLocker locker(&mMutex);
            while (mSendDataList.size() > 0) {
                if(mIsOver) break;
                QByteArray data = mSendDataList.takeFirst();
                if(socket)
                {
                    int size = socket->writeDatagram(data, QHostAddress(mHost), mPort);
                    if(mDebug)qDebug()<<"write data to host:"<<mHost<<mPort<<" with size:"<<size;
                }
            }
        }
        //接收数据
        bool data_recv = false;
        for(int i=0; i<10; i++)
        {
            if(mIsOver) break;
            if(socket->waitForReadyRead(1 * 1000))
            {
                error_num = 0;
                while (socket->hasPendingDatagrams())
                {
                    mLastRecvTime = QDateTime::currentDateTime();
                    QByteArray datagram;
                    qint64 dataSize = socket->pendingDatagramSize();
                    datagram.resize(dataSize);
                    /*if(mDebug)*/  qDebug()<<"recv data from socket:"<<dataSize<<socket->localAddress()<<mHost<<QThread::currentThread();
                    // 接收数据报，将其存放到datagram中
                    socket->readDatagram(datagram.data(), dataSize);
                    emit signalSendRecvData(datagram);
                }
                data_recv = true;
                break;
            }
        }
        if((!data_recv) && (!mIsOver))
        {
            qDebug()<<"no data from socket:"<<socket->localAddress()<<mHost<<QThread::currentThread();
            error_num++;
            if(error_num == 2)
            {
                emit signalNoDataRecv();
                break;
            }
        }
    }
    if(socket)
    {
        socket->abort();
        delete socket;
    }
}

bool zchxMulticastDataScoket::isFine() const
{
    return mLastRecvTime.secsTo(QDateTime::currentDateTime()) <= 10;
}

QUdpSocket* zchxMulticastDataScoket::init()
{
    if(mHost.length() == 0 || mPort == 0) return 0;
    QUdpSocket* socket = new QUdpSocket;
    //选择网卡
    if(!socket->bind(QHostAddress(mLocalIP), mPort,QAbstractSocket::ShareAddress))
    {
        qDebug()<<"bind port failed:"<<mPort<<" with local card:"<<mLocalIP;
        emit signalNoDataRecv();
        delete socket;
        return 0;
    }

    socket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, 0);//禁止本机接收
    socket->setMulticastInterface(mNIF);
    if(!socket->joinMulticastGroup(QHostAddress(mHost), mNIF))
    {
        qDebug()<<"joinMuticastGroup host failed:"<<mHost<<" with local card:"<<mLocalIP;
        emit signalNoDataRecv();
        delete socket;
        return 0;
    }
    qDebug()<<"init multicast succeed."<<mHost<<":"<<mPort<<" with local card:"<<mLocalIP;
    return socket;
}

void zchxMulticastDataScoket::processRecvData(const QByteArray &data)
{

}

#endif
