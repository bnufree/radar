#include "zchxrequestworker.h"
#include "zmq.h"
#include "zmq.hpp"
#include <QTimer>
#include "zchxdatadef.h"
#include <QDebug>
#include <QTextCodec>
#include "zchxcommonutils.h"

zchxRequestWorker::zchxRequestWorker(QObject *parent) : QObject(parent), mHeartTimer(0)
{
    mIsLogin = false;
    mCtx = zmq_ctx_new();
    mSocket = 0;
    mHeartTimer = new QTimer;
    mHeartTimer->setInterval(30 * 1000);
    connect(mHeartTimer, SIGNAL(timeout()), this, SLOT(slotHeartMsg()));
    connect(this, SIGNAL(signalSetHostInfo(QString,int)),
            this, SLOT(slotSetHostInfo(QString,int)));
    connect(this, SIGNAL(signalRequestCfgInfo()),
            this, SLOT(slotRequestCfgInfo()));
    connect(this, SIGNAL(signalSetRadarCtrl(QJsonValue)), this, SLOT(slotSetRadarCtrl(QJsonValue)));
    connect(this, SIGNAL(signalUpdateCfg(int,QJsonValue)), this, SLOT(slotUpdateCfg(int,QJsonValue)));

    if(!parent)
    {
        this->moveToThread(&mWorkThread);
        mWorkThread.start();
    }
    mHeartTimer->start();

}

zchxRequestWorker::~zchxRequestWorker()
{
    if(mHeartTimer) delete mHeartTimer;
    qDebug()<<"start???????????";
    if(mSocket) zmq_close(mSocket);
    if(mCtx) zmq_ctx_destroy(mCtx);
    qDebug()<<"now start termionate thread.....";
    if(mWorkThread.isRunning())mWorkThread.quit();
    qDebug()<<"end!!!!!!!!!";
}

void zchxRequestWorker::slotSetHostInfo(const QString &ip, int port)
{
    mHost = ip;
    mPort = port;
    if(mSocket) zmq_close(mSocket);
    if((mSocket = zmq_socket(mCtx, ZMQ_REQ)) == NULL)
    {
        emit signalSendErrorMsg(QString::fromUtf8("创建ｓｏｃｋｅｔ失败"));
        return;
    }

    //设置等待时间
    int timeout = 10 * 1000; //10s超时限制，没有收到消息就退出
    zmq_setsockopt(mSocket, ZMQ_RCVTIMEO, &timeout, sizeof(timeout));
    zmq_setsockopt(mSocket, ZMQ_SNDTIMEO, &timeout, sizeof(timeout));
    timeout = 0;
    zmq_setsockopt(mSocket, ZMQ_LINGER, &timeout, sizeof(timeout));
    //开始连接
    QString url = QString("tcp://%1:%2").arg(ip).arg(port);
    const char* sAddress = zchxCommonUtils::QString2Char(url);
    qDebug()<<"address:"<<sAddress;
    int sts = zmq_connect(mSocket, sAddress);
    if(sts != 0)
    {
        qDebug()<<"connect to server failed .code:"<<zmq_errno()<<" str:"<<zmq_strerror(zmq_errno())<<sAddress;
        emit signalSendErrorMsg(QString::fromUtf8("网络初始化失败%1").arg(zmq_strerror(zmq_errno())));
        emit signalRecheckHostPort(mHost, mPort);
    } else
    {
        qDebug()<<"connect to server succeed."<<sAddress;
        emit signalSendErrorMsg(QString::fromUtf8("网络初始化成功,开始请求服务器配置信息"));
        //开始请求服务器配置信息
        emit signalRequestCfgInfo();
    }
    delete []sAddress;

}

QString zchxRequestWorker::now()
{
    return QString::number(QDateTime::currentMSecsSinceEpoch());
}

bool zchxRequestWorker::sendMsgAndWaitReply(QJsonValue* ret_val, int cmd, const QJsonValue &val)
{
    //消息发送
    if(!mSocket) return false;
    QJsonObject obj;
    obj.insert(JSON_CMD, cmd);
    obj.insert(JSON_VAL, val);
    QJsonDocument doc(obj);
    QByteArray time =  now().toUtf8();
    QByteArray msg = doc.toJson();
    int size = 0;
    size += zmq_send(mSocket, time.constData(), time.size(), ZMQ_SNDMORE);
    size += zmq_send(mSocket, msg.constData(), msg.size(), 0);    
    qDebug()<<"send:"<<msg<<" and returned size:"<<size;
    if(size == 0) return false;
    //消息接收
    QByteArrayList recvlist;
    while (1) {
        int64_t more = 0;
        size_t more_size = sizeof(more);
        zmq_msg_t msg;
        zmq_msg_init(&msg);
        zmq_recvmsg(mSocket, &msg, 0);
        QByteArray bytes = QByteArray((char*)zmq_msg_data(&msg),(int)(zmq_msg_size(&msg)));
        zmq_msg_close(&msg);
        if(bytes.length() > 0)
        {
            recvlist.append(bytes);
        }
        zmq_getsockopt (mSocket, ZMQ_RCVMORE, &more, &more_size);
        if (more == 0) break;
    }
    if(recvlist.size() == 0)
    {
        qDebug()<<"recv data timeout...";
        return false;
    }
    qDebug()<<"recv data from server size:"<<recvlist.size();
    if(recvlist.size() < 2)
    {
        qDebug()<<"less than 2 data recved..."<<recvlist.first();
        return false;
    }
    if(1)
    {
        int index = 0;
        foreach (QByteArray byte, recvlist) {
            qDebug()<<"byte["<<(index++)<<"]="<<byte;
        }
    }

    if(recvlist.size() > 2)
    {
        qDebug()<<"more than 2 data recved...";
        return false;
    }

    //检查返回的信息
    qint64 recv_time = recvlist.first().toLongLong();
    doc = QJsonDocument::fromJson(recvlist.last());
    if(!doc.isObject())
    {
        qDebug()<<"recv data is not json format.";
        return false;
    }
    obj = doc.object();
    qint64 request_time = obj.value(JSON_REQUEST_TIME).toVariant().toLongLong();
    int    ret_cmd = obj.value(JSON_CMD).toInt();
    int   status = obj.value(JSON_STATUS).toInt();
    if(cmd == ret_cmd && request_time == time.toLongLong())
    {
        //同一条命令的返回信息
        if(status)
        {
            if(ret_val) *ret_val = obj.value(JSON_VAL);
            return true;
        }
        qDebug()<<"recv error form server:"<<obj.value(JSON_STATUS_STR);
        return false;
    }

    qDebug()<<"recv cmd and time is not request one"<<ret_cmd<<cmd<<request_time<<time;
    return false;
}

void zchxRequestWorker::slotHeartMsg()
{
    if(!mIsLogin) return;
    static int heart_error = 0;
    if(!sendMsgAndWaitReply(0, zchxCommon::Msg_Heart, "HEARTHEARTHEART"))
    {
        heart_error++;
        emit signalSendErrorMsg(QString::fromUtf8("心跳失败%1次").arg(heart_error));
        if(heart_error == 5)
        {
            //开始重连
            mIsLogin = false;
            emit signalSetHostInfo(mHost, mPort);
        }
        return;
    }
    heart_error = 0;
}

void zchxRequestWorker::slotRequestCfgInfo()
{
    QJsonValue ret_val;
    static int login_errno = 0;
    if(!sendMsgAndWaitReply(&ret_val, zchxCommon::Msg_Request_Cfg, "CFGCFGCFG"))
    {
        emit signalSendErrorMsg(QString::fromUtf8("请求配置信息失败，正在重试..."));
        login_errno++;
        if(login_errno == 5)
        {
            emit signalRecheckHostPort(mHost, mPort);
        } else
        {            
            QThread::sleep(5);
            mIsLogin = false;
            emit signalSetHostInfo(mHost, mPort);
        }
        return;
    }

    login_errno = 0;

    mIsLogin = true;
    emit signalSendSvrCfg(ret_val.toObject());
}

void zchxRequestWorker::slotUpdateSvrCfg(const QJsonValue& val)
{
//    QJsonValue ret_val;
//    if(!sendMsgAndWaitReply(&ret_val, zchxCommon::Msg_Update_Cfg, val))
//    {
//        emit signalSendErrorMsg(QString::fromUtf8("更新配置信息失败"));
//    } else
//    {
//        emit signalSendSvrCfg(ret_val.toObject());
//    }
}

void zchxRequestWorker::slotUpdateCfg(int cmd, const QJsonValue &val)
{
    QJsonValue ret_val;
    if(!sendMsgAndWaitReply(&ret_val, cmd, val))
    {
        emit signalSendErrorMsg(QString::fromUtf8("更新雷达信息失败"));
    } else
    {
        emit signalSendErrorMsg(QString::fromUtf8("更新雷达信息成功"));
    }
}

void zchxRequestWorker::slotSetRadarCtrl(const QJsonValue &val)
{
    QJsonValue ret_val;
    if(!sendMsgAndWaitReply(&ret_val, zchxCommon::Msg_Update_RadarCtrl, val))
    {
        emit signalSendErrorMsg(QString::fromUtf8("更新雷达控制信息失败"));
    } else
    {
        emit signalSendErrorMsg(QString::fromUtf8("更新雷达控制信息成功"));
    }
}

