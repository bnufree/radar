#ifndef ZCHXRADARVIDEODATACHANGE_H
#define ZCHXRADARVIDEODATACHANGE_H

#include <QObject>
#include "zchxdatadef.h"
#include "zchxrecvutils.h"

class ZCHXRadarDataChange : public QObject
{
    Q_OBJECT
public:
    explicit ZCHXRadarDataChange(QObject *parent = 0);
    ~ZCHXRadarDataChange();
    void stop();
    void appendPublishSetting(const QString& host, const zchxCommon::zchxPublishSettingsList& list);
signals:
    void signalClearAllEcdisData();
    void signalConnectedStatus(bool sts, const QString& msg, const QString& topic);
    void signalRecvDataNow(int id,  int length);
    void signalSendAisDataList(const QList<ZCHX::Data::ITF_AIS>& list);
    void signalSendRadarPoint(const QMap<int, QList<ZCHX::Data::ITF_RadarPoint> >&);
    void signalSendLimitDataList(const QList<ZCHX::Data::ITF_IslandLine>&);
    void signalSendRadarVideo(const QList<ZCHX::Data::ITF_RadarVideoImage>&);
    void signalSendRadarReport(const zchxCommon::zchxRadarReportList& report);
    void signalSendSvrCfg(const zchxCommon::zchxRadarServerCfg& cfg);
    void signalSendNewPublishSetting(const zchxCommon::zchxPublishSettingsList& list);
    void signalSendNewDevList(const zchxCommon::zchxRadarDeviceList& list);
    void signalUpdateRadarChannelRadius(int radar, int channel, double radius, double rangefactor);
    void signalUpdatePublishPortStatus(const zchxCommon::zchxPublishSettingsList& list);
    void signalUpdateRadarType(const zchxCommon::zchxRadarTypeData& type);


private:
    QList<ZCHXReceiverThread*> mThreadList;
};

#endif // ZCHXRADARVIDEODATACHANGE_H
