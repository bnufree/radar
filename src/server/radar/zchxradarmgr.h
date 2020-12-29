#ifndef ZCHXRADARMGR_H
#define ZCHXRADARMGR_H

#include <QObject>
#include "zchxdatadef.h"
#include "zchxradardevice.h"
#include "zchxfilterareafile.h"

class zchxRadarMgr : public QObject
{
    Q_OBJECT
public:
    explicit zchxRadarMgr(bool filter, QObject *parent = 0);
    ~zchxRadarMgr();
public slots:
    void     updateDeviceList(const zchxCommon::zchxRadarDeviceList& list);
    void     updateFilterArea(int cmd, const zchxCommon::zchxfilterAreaList &area);
    void     updateDevFilterStatus(bool sts);
    void     setControl(const zchxCommon::zchxRadarCtrlDef& def);
signals:
    void    signalSendRecvIP(const QString& ip, int id);
    void    signalSendHeadChangedData(int radar, int channel, int head);
public slots:
    void     slotSendFilterAreaData();
private:
    zchxCommon::zchxRadarDeviceList                         mDevList;
    QMap<int, QSharedPointer<ZCHXRadarDevice>>          mDevMgr;
    zchxFilterAreaFile*                                     mFilterMgr;
    bool                                                    mFilterEnabled;

};

#endif // ZCHXRADARMGR_H
