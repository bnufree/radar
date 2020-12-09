#include "zchxradaroutputdatamgr.h"
#include "dataout/zchxdataoutputservermgr.h"
#include "zchxradarcommon.h"
#include <QTimer>
#include "zchxradarmergethread.h"
#include <QDebug>

zchxRadarOutputDataMgr::zchxRadarOutputDataMgr(QObject *parent) :
    QObject(parent)
{
    mMergeThread = new zchxRadarMergeThread;    
    mMergeThread->start();
}

zchxRadarOutputDataMgr::~zchxRadarOutputDataMgr()
{
    if(mMergeThread)
    {
        mMergeThread->setOver();
        mMergeThread->deleteLater();
    }
}

void zchxRadarOutputDataMgr::updateTracks(int channel, const zchxRadarSurfaceTrack &track)
{
    mTrackMap[channel] = track;
    //移除超时很久未更新的通道
    qint64 last_time = track.utc();
    QList<int> keys = mTrackMap.keys();
    foreach (int key, keys) {
        if(last_time - mTrackMap[key].utc() >= 5000)
        {
            mTrackMap.remove(key);
        }
    }
    if(mMergeThread) mMergeThread->appendTask(mTrackMap);
}

void zchxRadarOutputDataMgr::updateVideoImage(int channel, const zchxRadarVideoSingleImage &img)
{
    //直接发送所有通道的回波图片
    mVideoImageMap[channel] = img;
    zchxRadarVideoImages *res = new zchxRadarVideoImages;
    res->set_utc(QDateTime::currentMSecsSinceEpoch());
    foreach (zchxRadarVideoSingleImage img, mVideoImageMap) {
        res->add_images()->CopyFrom(img);
    }
    qDebug()<<"res img size:"<<res->images_size();
    if(res->images_size() > 0)
    {
        emit PUBLISH_INS->signalSendPublishData(zchxCommon::THREAD_DATA_RADAR_VIDEO, zchxRadarUtils::protoBufMsg2ByteArray(res));
    }
    delete res;
}

void zchxRadarOutputDataMgr::updateChannelReport(int channel, const zchxCommon::zchxRadarChannelReport &report)
{
    mChannelReport[channel] = report;
    zchxCommon::zchxRadarReportList list(mChannelReport.values());
    emit PUBLISH_INS->signalSendPublishData(zchxCommon::THREAD_DATA_RADAR_REPORT, QJsonDocument(list.toJson().toArray()).toJson());
}


void zchxRadarOutputDataMgr::updateRadarType(int channel, int type, const QString &radarID)
{
    zchxCommon::zchxRadarTypeData data;
    data.radar_id = radarID;
    data.channel_id = channel;
    data.type = type;
    QJsonObject obj;
    obj.insert(JSON_CMD, zchxCommon::SVR_UPDATE_RADAR_TYPE);
    obj.insert(JSON_VAL, data.toJson());
    emit PUBLISH_INS->signalSendPublishData(zchxCommon::THREAD_DATA_SVRCFG, QJsonDocument(obj).toJson());
}

void zchxRadarOutputDataMgr::updateRadarRadiusAndFactor(int channel, double radius, double factor, const QString &radarID)
{
    QJsonObject obj;
    obj.insert(JSON_CMD, zchxCommon::SVR_UPDATE_RADAR_RADIUS);
    QJsonArray array;
    array.append(channel);
    array.append(radius);
    array.append(factor);
    array.append(radarID);
    obj.insert(JSON_VAL, array);
    emit PUBLISH_INS->signalSendPublishData(zchxCommon::THREAD_DATA_SVRCFG, QJsonDocument(obj).toJson());
}
