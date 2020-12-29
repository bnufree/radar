#include "zchxradardevice.h"
#include <QDebug>

ZCHXRadarDevice::ZCHXRadarDevice(const zchxCommon::zchxRadarDevice& device,
                                 bool filter,
                                 const zchxCommon::zchxfilterAreaList& list,
                                 QObject *parent) :
    QObject(parent),
    mFilterSts(filter),
    mFilterAreaList(list),
    mOuputMgr(new zchxRadarOutputDataMgr)

{
    qRegisterMetaType<zchxCommon::zchxRadarChannelReport>("const zchxCommon::zchxRadarChannelReport&");
    updateDev(device);
}

void ZCHXRadarDevice::stop()
{
    foreach (zchxCommon::zchxRadarChannel channel, mDev.channel_list) {
        stopChannel(channel.id);
    }
}

void ZCHXRadarDevice::stopChannel(int channel)
{
    mRadarChannelMap.remove(channel);
}

void ZCHXRadarDevice::updateFilterArea(bool sts, const zchxCommon::zchxfilterAreaList& list)
{
    mFilterSts = sts;
    mFilterAreaList = list;
    foreach (QSharedPointer<ZCHXRadarDataServer> ptr, mRadarChannelMap) {
        if(ptr)
        {
            ptr->updateFilterAreaSettings(sts, list);
        }
    }

}

void ZCHXRadarDevice::setCtrl(int ch, int type, QJsonValue val)
{
    //更新设定
    QSharedPointer<ZCHXRadarDataServer> svr = mRadarChannelMap[ch];
    if(svr)
    {
        svr->setCtrlValue(type, val);
    }
}

void ZCHXRadarDevice::updateDev(const zchxCommon::zchxRadarDevice &device)
{
    //检查是否有通道删除了
    QList<int> channel_id_list;
    for(int i=0; i<mDev.channel_list.size(); i++)
    {
        channel_id_list.append(mDev.channel_list[i].id);
    }
    //检查通道是否发生了变化
    mDev = device;
    zchxVideoParserSettings parse(mDev.base.name, mDev.base.id, mDev.base.center.lat, mDev.base.center.lon, mDev.parse_param.toJson().toObject());
    parse.filter_enable = mFilterSts;
    parse.filter_area_list = mFilterAreaList;
    for(int i=0; i<mDev.channel_list.size(); i++)
    {
        zchxCommon::zchxRadarChannel channel = mDev.channel_list[i];
        parse.channel_id = channel.id;
        startChannel(channel, parse);
        channel_id_list.removeOne(channel.id);
    }

    foreach (int id, channel_id_list) {
        stopChannel(id);
    }
}


ZCHXRadarDataServer* ZCHXRadarDevice::startChannel(const zchxCommon::zchxRadarChannel &channel,
                                                   const zchxVideoParserSettings& setting)
{
    ZCHXRadarDataServer* result = 0;
    if(mRadarChannelMap.contains(channel.id))
    {
        //更新设定
        QSharedPointer<ZCHXRadarDataServer> svr = mRadarChannelMap[channel.id];
        if(svr)
        {
            svr->updateChannelSettings(channel, setting);
            result = svr.data();
        }
    }
    if(!result)
    {
        QSharedPointer<ZCHXRadarDataServer> server(new ZCHXRadarDataServer(mOuputMgr, channel, setting, mDev.local_connect_ip));
        mRadarChannelMap.insert(channel.id, server);
        connect(server.data(), &ZCHXRadarDataServer::signalSendHeadChangedData, [=](int head){
            emit signalSendHeadChangedData(mDev.base.id, channel.id, head);
        });
    }

    return result;
}


ZCHXRadarDevice::~ZCHXRadarDevice()
{
    if(mOuputMgr) delete mOuputMgr;
    stop();
}

void ZCHXRadarDevice::slotOpenRadar(int channel)//打开雷达
{
    if(!mRadarChannelMap.contains(channel)) return;
    QSharedPointer<ZCHXRadarDataServer> server = mRadarChannelMap[channel];
    if(!server) return;
    server->slotOpenRadar();
}

void ZCHXRadarDevice::slotCloseRadar(int channel)//关闭雷达
{
    if(!mRadarChannelMap.contains(channel)) return;
    QSharedPointer<ZCHXRadarDataServer> server = mRadarChannelMap[channel];
    if(!server) return;
    server->slotCloseRadar();

}


void ZCHXRadarDevice::slotOutputData(int channel, bool a)//打印回波数据
{
    if(!mRadarChannelMap.contains(channel)) return;
    QSharedPointer<ZCHXRadarDataServer> server = mRadarChannelMap[channel];
    if(!server) return;
    server->slotPrintRecvData(a);
}



