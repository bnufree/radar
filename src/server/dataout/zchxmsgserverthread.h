#ifndef ZCHXMSGSERVERTHREAD_H
#define ZCHXMSGSERVERTHREAD_H

#include <QMutex>
#include "zchxdatathread.h"
#include "zchxdatadef.h"

class zchxCfgMgr;
class zchxMsgServerThread : public zchxDataThread
{
    Q_OBJECT
public:
    explicit zchxMsgServerThread(void* ctx, int port, QObject *parent = 0);
    ~zchxMsgServerThread();
    void     run();
    void     setCfgMgr(zchxCfgMgr* mgr){mCfgMgr = mgr;}

signals:
    void    signalPortStatus(const zchxCommon::zchxPortStatus& sts);
    void    signalSendEditFilterAreas(int cmd, const zchxCommon::zchxfilterAreaList&);
    void    signalSendCtrlFromClient(const zchxCommon::zchxRadarCtrlDef& cmd);
private:
    bool     processFilterAreaMsg(int cmd, const QJsonValue& val);
    bool     processRequestCfg(QJsonObject& obj);
    bool     processUpdateCfg(int cmd, const QJsonValue& val);

private:
    zchxCfgMgr*         mCfgMgr;
};

#endif // ZCHXDATAOUTPUTSERVERTHREAD_H
