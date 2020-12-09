#include "zchxdatathread.h"
#include "zmqmonitorthread.h"
#include "zchxcommonutils.h"
#include <QDebug>


zchxDataThread::zchxDataThread(int type, void* ctx, int port, bool monitor, QObject *parent)
    : QThread(parent)
    , mCtx(ctx)
    , mPort(port)
    , mIsOk(false)
    , mStop(false)
    , mSocket(0)
    , mMonitorThread(0)
    , mMonitorClient(monitor)
    , mZmqType(type)
{
    mUrl = QString("tcp://*:%1").arg(mPort);
    mIsOk = init();
    if(mIsOk && mMonitorClient)
    {
        //监听雷达目标zmq
        QString monitorTrackUrl = QString("inproc://zchx.monitor.%1").arg(QString::number(mPort));
        zmq_socket_monitor (mSocket, monitorTrackUrl.toStdString().c_str(), ZMQ_EVENT_ALL);
        mMonitorThread = new ZmqMonitorThread(mCtx, monitorTrackUrl, 0);
        connect(mMonitorThread, SIGNAL(finished()), mMonitorThread, SLOT(deleteLater()));
        mMonitorThread->start();
    }

    if(!isOK())
    {
        qDebug()<<"error init occured."<<zmq_errno()<<zmq_strerror(zmq_errno())<<mPort;
    }
}

zchxDataThread::~zchxDataThread()
{
    stopMe();

    wait();
    if(mMonitorThread)
    {
        mMonitorThread->quit();
    }
    if(mSocket)
    {
        zmq_unbind(mSocket, mUrl.toLatin1().constData());
        zmq_close(mSocket);
        qDebug()<<"socket closed..."<<mUrl;
    }
}

bool zchxDataThread::init()
{
    if(mCtx == 0)
    {
        qDebug()<<"Context is null. invalid....";
        return false;
    }

    mSocket = zmq_socket(mCtx, mZmqType);
    if(mSocket == 0)
    {
        qDebug()<<"create socket error";
        return false;
    }
    const char* addr = zchxCommonUtils::QString2Char(mUrl);
    int err_num = 0;
    while (1) {
        int sts = zmq_bind(mSocket, addr);//
        if(sts != 0)
        {
             err_num++;
            if(err_num == 10)
            {
                qDebug()<<"bind server to port:"<<mPort<<" failed..."<<" url:"<<QString::fromStdString(addr);
                break;
            } else
            {
                msleep(1000);
            }
        } else
        {
            err_num = 0;
            break;
        }
    }
    if(err_num == 0)qDebug()<<"bind server to port:"<<mPort<<" success..."<<" url:"<<QString::fromStdString(addr);
    return err_num == 0;
}
