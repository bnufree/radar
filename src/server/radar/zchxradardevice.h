#ifndef ZCHXRADARDEVICE_H
#define ZCHXRADARDEVICE_H

#include <QObject>
#include "zchxradardataserver.h"
#include <QSharedPointer>
#include "zchxradaroutputdatamgr.h"

class ZCHXRadarDevice: public QObject
{
    Q_OBJECT
public:
    explicit ZCHXRadarDevice(const zchxCommon::zchxRadarDevice& device, bool filter, const zchxCommon::zchxfilterAreaList& list, QObject *parent = 0);
    ~ZCHXRadarDevice();
    void  updateDev(const zchxCommon::zchxRadarDevice& device);
    void  stopChannel(int channel);
    ZCHXRadarDataServer* startChannel(const zchxCommon::zchxRadarChannel& channel, const zchxVideoParserSettings& setting);
    void  stop();
    void  updateFilterArea(bool sts, const zchxCommon::zchxfilterAreaList& list);
    int   id() const {return mDev.base.id;}
    void  setCtrl(int ch, int type, QJsonValue val);

signals:
    void signalSendRecvedContent(qint64 time, const QString& name, const QString& content);
    void startProcessSignal();
    void prtVideoSignal_1(bool);
    void joinGropsignal(QString);
    void signalRadarConfigChanged(int, int, int);
    void colorSetSignal(int,int,int,int,int,int);//回波颜色设置
    void signalSendRadarType(int channel, int type, const QString& radarID);
    void signalSendRecvIP(const QString& ip);
    void signalSendHeadChangedData(int radar, int channel, int head);

public slots:
    void slotOutputData(int channel, bool sts);//打印回波数据
    void slotOpenRadar(int channel);
    void slotCloseRadar(int channel);

private:
    zchxCommon::zchxRadarDevice                             mDev;
    QMap<int, QSharedPointer<ZCHXRadarDataServer>>          mRadarChannelMap;
    bool                                                    mFilterSts;
    zchxCommon::zchxfilterAreaList                          mFilterAreaList;
    zchxRadarOutputDataMgr*                                 mOuputMgr;
};

#endif // ZCHXRADARDEVICE_H
