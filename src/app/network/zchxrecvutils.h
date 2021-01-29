#ifndef ZCHXRADARUTILS_H
#define ZCHXRADARUTILS_H

#include "qt/zchxutils.hpp"
#include <QThread>
#include "zchxdatadef.h"


struct DataRecvPair
{
    int         data_id;
    QString     topic;

    bool operator ==(const DataRecvPair& other) const
    {
        return this->topic == other.topic;
    }
};

class ZCHXReceiverThread : public QThread
{
    Q_OBJECT
public:
    explicit ZCHXReceiverThread(const QString& host, int port, const QList<DataRecvPair>& cmd_list, QObject *parent = 0);
    virtual ~ZCHXReceiverThread();
    virtual void run();

    bool getIsOver() const {return mIsOver;}
    void setIsOver(bool value) {mIsOver = value;}
    virtual void parseRecvData(const QByteArrayList& data);
    bool connectToHost();
    void disconnectToHost();
private:
    void parseAis(const QByteArray& bytes);
    void parseRadarTrack(const QByteArray& bytes);
    void parseRadarVideo(const QByteArray& bytes);
    void parseRadarReport(const QByteArray& bytes);
    void parseRadarFilterArea(const QByteArray& bytes);
    void parseSvrCfg(const QByteArray& bytes);
    void parseRadarNodeLog(const QByteArray& bytes);
signals:
    void signalConnectedStatus(bool sts, const QString& msg, const QString& topic);
    void signalRecvDataNow(int id,  int length);
    void signalSendAisDataList(const QList<ZCHX::Data::ITF_AIS>& list);
    void signalSendRadarPoint(const QMap<int, QList<ZCHX::Data::ITF_RadarPoint> >&);
    void signalSendLimitDataList(const QList<ZCHX::Data::ITF_IslandLine>&);
    void signalSendRadarVideo(const QList<ZCHX::Data::ITF_RadarVideoImage>&);
    void signalSendRadarNodeLog(const QList<ZCHX::Data::ITF_RadarNodeLog>&);
    void signalSendRadarReport(const zchxCommon::zchxRadarReportList& report);
    void signalSendSvrCfg(const zchxCommon::zchxRadarServerCfg& cfg);
    void signalSendNewPublishSetting(const zchxCommon::zchxPublishSettingsList& list);
    void signalSendNewDevList(const zchxCommon::zchxRadarDeviceList& list);
    void signalUpdateRadarChannelRadius(int radar, int channel, double radius, double rangefactor);
    void signalUpdatePublishPortStatus(const zchxCommon::zchxPublishSettingsList& list);
    void signalUpdateRadarType(const zchxCommon::zchxRadarTypeData& type);


protected:
    QList<DataRecvPair>     mCmdList;
    QString                 mHost;
    int                     mPort;
    bool                    mIsOver;
    void                    *mCtx;
    void                    *mSocket;
    bool                    mIsConnect;
    QString                 mUrl;
    QStringList             mTopicList;
};





#endif // ZCHXRADARUTILS_H
