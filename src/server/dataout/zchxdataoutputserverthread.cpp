#include "zchxdataoutputserverthread.h"
#include <QDebug>
#include <QDateTime>

zchxDataOutputServerThread::zchxDataOutputServerThread(void* ctx, int port, QObject *parent)
    : zchxDataThread(ZMQ_PUB, ctx, port, false, parent)
{
}

zchxDataOutputServerThread::~zchxDataOutputServerThread()
{

}

void zchxDataOutputServerThread::run()
{
    while (!mStop)
    {
        //获取当前的任务
        zchxSendTaskList sendList;
        {
            QMutexLocker locker(&mMutex);
            if(mSendContentList.size() > 0)
            {
                sendList.append(mSendContentList);
            }
            mSendContentList.clear();
        }
        if(sendList.size() > 0 && isOK())
        {
            foreach (zchxSendTask task, sendList)
            {
                //数据分成3帧进行发送(时间+topic+内容)  
                QByteArray sTimeArray = QString::number(QDateTime::currentMSecsSinceEpoch()).toUtf8();
//                qDebug()<<"send data time:"<<QDateTime::fromMSecsSinceEpoch(sTimeArray.toLongLong())<<task[0]<<task[1].size();
                zmq_send(mSocket, task[0].data(), task[0].size(), ZMQ_SNDMORE);
                zmq_send(mSocket, sTimeArray.data(), sTimeArray.size(), ZMQ_SNDMORE);
                zmq_send(mSocket, task[1].data(), task[1].size(), 0);
            }
        }

        msleep(100);
    }
}

void zchxDataOutputServerThread::slotRecvContents(const QByteArray& content, const QString& topic)
{
    QMutexLocker locker(&mMutex);
    zchxSendTask task;
    task.append(topic.toUtf8());
    task.append(content);
    mSendContentList.append(task);
    if(!mTopicList.contains(topic)) mTopicList.append(topic);
}
