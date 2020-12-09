#include "zchxradardatachange.h"
//#include "zchxradarpointthread.h"
//#include "zchxradarechothread.h"
//#include "zchxaisthread.h"
//#include "zchxradarlimitareathread.h"
#include<QDebug>

ZCHXRadarDataChange::ZCHXRadarDataChange(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<QList<ZCHX::Data::ITF_AIS>>("const QList<ZCHX::Data::ITF_AIS>&");
    qRegisterMetaType<QList<ZCHX::Data::ITF_RadarPoint>>("const QList<ZCHX::Data::ITF_RadarPoint>&");
    qRegisterMetaType<QMap<int, QList<ZCHX::Data::ITF_RadarPoint> > >("const QMap<int, QList<ZCHX::Data::ITF_RadarPoint> >&");
    qRegisterMetaType<QList<ZCHX::Data::ITF_IslandLine>>("const QList<ZCHX::Data::ITF_IslandLine>&");
    qRegisterMetaType<QList<ZCHX::Data::ITF_RadarVideoImage>>("const QList<ZCHX::Data::ITF_RadarVideoImage>&");
    qRegisterMetaType<zchxCommon::zchxRadarReportList>("const zchxCommon::zchxRadarReportList&");
    qRegisterMetaType<zchxCommon::zchxRadarServerCfg>("const zchxCommon::zchxRadarServerCfg&");

    qRegisterMetaType<zchxCommon::zchxPublishSettingsList>("const zchxCommon::zchxPublishSettingsList&");
    qRegisterMetaType<zchxCommon::zchxRadarDeviceList>("const zchxCommon::zchxRadarDeviceList&");
    qRegisterMetaType<zchxCommon::zchxPublishSettingsList>("const zchxCommon::zchxPublishSettingsList&");
    qRegisterMetaType<zchxCommon::zchxRadarTypeData>("const zchxCommon::zchxRadarTypeData&");
}

ZCHXRadarDataChange::~ZCHXRadarDataChange()
{
    stop();
}

void ZCHXRadarDataChange::stop()
{
    foreach (ZCHXReceiverThread* thread, mThreadList) {
        thread->setIsOver(true);
        thread->deleteLater();
    }
}

void ZCHXRadarDataChange::appendPublishSetting(const QString& host, const zchxCommon::zchxPublishSettingsList &list)
{
    //统计每一个端口对应的topic的数目，然后归集每一个端口的所有的topic设定数据过滤
    QMap<int, QList<DataRecvPair>> port_topic_list;
    for(int i=0; i<list.size(); i++)
    {
        zchxCommon::zchxPublishSetting set = list[i];
        DataRecvPair pair = {set.id, set.topic};
        QList<DataRecvPair>& temp_list = port_topic_list[set.port];
        if(!temp_list.contains(pair)) temp_list.append(pair);
    }
    //开始创建线程，接收数据
    emit signalClearAllEcdisData();
    stop();
    QMap<int, QList<DataRecvPair>>::iterator it = port_topic_list.begin();
    for(; it != port_topic_list.end(); it++)
    {
        ZCHXReceiverThread* thread = new ZCHXReceiverThread(host, it.key(), it.value());
        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        connect(thread, SIGNAL(signalConnectedStatus(bool,QString,QString)), this, SIGNAL(signalConnectedStatus(bool,QString,QString)));
        connect(thread, SIGNAL(signalSendAisDataList(QList<ZCHX::Data::ITF_AIS>)), this, SIGNAL(signalSendAisDataList(QList<ZCHX::Data::ITF_AIS>)));
        connect(thread, SIGNAL(signalSendLimitDataList(QList<ZCHX::Data::ITF_IslandLine>)), this, SIGNAL(signalSendLimitDataList(QList<ZCHX::Data::ITF_IslandLine>)));
        connect(thread, SIGNAL(signalSendRadarPoint(QMap<int,QList<ZCHX::Data::ITF_RadarPoint> >)),
                this, SIGNAL(signalSendRadarPoint(QMap<int,QList<ZCHX::Data::ITF_RadarPoint> >)));
        connect(thread, SIGNAL(signalSendRadarReport(zchxCommon::zchxRadarReportList)), this, SIGNAL(signalSendRadarReport(zchxCommon::zchxRadarReportList)));
        connect(thread, SIGNAL(signalSendRadarVideo(QList<ZCHX::Data::ITF_RadarVideoImage>)), this, SIGNAL(signalSendRadarVideo(QList<ZCHX::Data::ITF_RadarVideoImage>)));
        connect(thread, SIGNAL(signalSendSvrCfg(zchxCommon::zchxRadarServerCfg)), this, SIGNAL(signalSendSvrCfg(zchxCommon::zchxRadarServerCfg)));
        connect(thread, SIGNAL(signalSendNewDevList(zchxCommon::zchxRadarDeviceList)), this, SIGNAL(signalSendNewDevList(zchxCommon::zchxRadarDeviceList)));
        connect(thread, SIGNAL(signalSendNewPublishSetting(zchxCommon::zchxPublishSettingsList)), this, SIGNAL(signalSendNewPublishSetting(zchxCommon::zchxPublishSettingsList)));
        connect(thread, SIGNAL(signalUpdateRadarChannelRadius(int,int,double,double)), this, SIGNAL(signalUpdateRadarChannelRadius(int,int,double,double)));
        connect(thread, SIGNAL(signalUpdatePublishPortStatus(zchxCommon::zchxPublishSettingsList)), this, SIGNAL(signalUpdatePublishPortStatus(zchxCommon::zchxPublishSettingsList)));
        connect(thread, SIGNAL(signalUpdateRadarType(zchxCommon::zchxRadarTypeData)), this, SIGNAL(signalUpdateRadarType(zchxCommon::zchxRadarTypeData)));

        thread->start();
        mThreadList.append(thread);
    }

}
