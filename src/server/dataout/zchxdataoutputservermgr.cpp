#include "zchxdataoutputservermgr.h"
#include "zchxdataoutputserverthread.h"
#include "zmq.h"
#include <QTimer>
#include <QDebug>
#include <QTimer>
//#include "common.h"

zchxDataOutputServerMgr* zchxDataOutputServerMgr::m_pInstace = 0;



zchxDataOutputServerMgr::zchxDataOutputServerMgr(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<zchxCommon::zchxPublishSettingsList>("const zchxCommon::zchxPublishSettingsList&");
    qRegisterMetaType<zchxCommon::zchxPortStatusList>("const zchxCommon::zchxPortStatusList&");

    mCtx = zmq_ctx_new();

    connect(this, SIGNAL(signalInitPublish(zchxCommon::zchxPublishSettingsList)),
            this, SLOT(initFromCfg(zchxCommon::zchxPublishSettingsList)));
    connect(this, SIGNAL(signalSendPublishData(int,QByteArray)),
            this, SLOT(slotAppendData(int,QByteArray)));

    moveToThread(new QThread);
    if(!this->thread()->isRunning()) this->thread()->start();
}

zchxDataOutputServerMgr* zchxDataOutputServerMgr::instance()
{
    if(m_pInstace == 0) m_pInstace = new zchxDataOutputServerMgr;
    return m_pInstace;
}

zchxDataOutputServerMgr::~zchxDataOutputServerMgr()
{ 
    foreach (zchxDataOutputServerThread* thread, mPortThreadList) {
        if(thread){
            delete thread;
        }
    }
    if(mCtx) zmq_ctx_destroy(mCtx);
}

void zchxDataOutputServerMgr::initFromCfg(const zchxCommon::zchxPublishSettingsList& list)
{
    for(int i=0; i<list.size(); i++)
    {
        zchxCommon::zchxPublishSetting setting = list[i];
        qDebug()<<setting.id<<setting.port<<setting.topic;
        QString old_topic = mCaseTopicList.value(setting.id, "");

        bool create_new_thread = true;
        //检查事件的对象是否已经有了线程，事件的ID是不会变化的
        zchxDataOutputServerThread *old_case_thread = mCaseThreadList.value(setting.id, 0);
        if(old_case_thread)
        {
            //事件已经创建了线程，检查就的线程的端口号是否与新设定的相同
            if(old_case_thread->getPort() == setting.port)
            {
                if(old_topic != setting.topic)
                {
                    old_case_thread->removeTopic(old_topic);
                    old_case_thread->appendTopic(setting.topic);
                    create_new_thread = false;
                }
            } else
            {
                //端口号发生了变化，从旧的线程移除
                old_case_thread->removeTopic(old_topic);
            }
        }

        if(create_new_thread)
        {
            old_case_thread = getThread(setting.port, setting.topic);
        }

        mCaseTopicList[setting.id] = setting.topic;
        mCaseThreadList[setting.id] = mPortThreadList[setting.port];
    }

    zchxCommon::zchxPortStatusList stslist;
    foreach (zchxDataOutputServerThread* thread, mPortThreadList) {
        if(thread)
        {
            qDebug()<<thread->getPort()<<thread->topic();
            if(thread->topic().size() == 0)
            {
                delete thread;
            } else
            {
                zchxCommon::zchxPortStatus data;
                data.port = thread->getPort();
                data.sts = thread->isOK();
                data.topic = thread->topic().join(" ");
                stslist.append(data);
            }
        }
    }

    qDebug()<<mCaseTopicList;
    emit signalSendPortStartStatus(stslist);
}

void zchxDataOutputServerMgr::slotAppendData(int caseid, const QByteArray& data)
{
    zchxDataOutputServerThread* thread = mCaseThreadList.value(caseid, 0);
    if(thread)
    {
        QString topic = mCaseTopicList[caseid];
//        qDebug()<<"case id:"<<caseid<<" topic:"<<topic;
        thread->slotRecvContents(data, topic);
        if(!thread->topic().contains(topic))
        {
            zchxCommon::zchxPortStatus data;
            data.port = thread->getPort();
            data.sts = thread->isOK();
            data.topic = thread->topic().join(" ");
            emit signalSendPortStartStatus(zchxCommon::zchxPortStatusList(data));
        }

    }
}

zchxDataOutputServerThread* zchxDataOutputServerMgr::getThread(int port, const QString& topic)
{
    zchxDataOutputServerThread* thread = mPortThreadList.value(port, 0);
    if(!thread)
    {
        thread = new zchxDataOutputServerThread(mCtx, port);
        mPortThreadList[port] = thread;
    }
    if(!thread->topic().contains(topic))
    {
        thread->appendTopic(topic);
    }
    if(!thread->isRunning()) thread->start();

    return thread;
}

void zchxDataOutputServerMgr::appendAisData(const QByteArray &data)
{
    slotAppendData(zchxCommon::THREAD_DATA_AIS, data);
}

void zchxDataOutputServerMgr::appendFilterData(const QByteArray &data)
{
//    slotAppendData(zchxCommon::THREAD_DATA_FILTER, data);
}

void zchxDataOutputServerMgr::appendTrackData(const QByteArray &data)
{
    slotAppendData(zchxCommon::THREAD_DATA_RADAR_TRACK, data);
}

void zchxDataOutputServerMgr::appendVideoData(const QByteArray &data)
{
    slotAppendData(zchxCommon::THREAD_DATA_RADAR_VIDEO, data);
}
