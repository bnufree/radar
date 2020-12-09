#ifndef MAINPROCESS_H
#define MAINPROCESS_H

#include <QObject>
#include <QJsonDocument>
#include "zchxdatadef.h"
#include "dataout/zchxmsgserverthread.h"

#define MainProc        MainProcess::instance()

class ZCHXAnalysisAndSendRadar;
class zchxCfgMgr;
class zchxRadarMgr;

class MainProcess : public QObject
{
    Q_OBJECT
private:
    explicit MainProcess(const QString& fileName = QString(), QObject *parent = 0);
    ~MainProcess();
    bool    isDataServerOk();

public:
    static MainProcess* instance();

public slots:
    bool    start();
    bool    updateFilterAreaMsg(int cmd, const zchxCommon::zchxfilterAreaList& area);
    void    updateFilterStatus(bool sts);
    void    slotRecvPublishPortStartStatus(const zchxCommon::zchxPortStatusList& list);
    void    slotRecvMsgPortStatus(const zchxCommon::zchxPortStatus& sts);
    void    slotRecvCtrlFromServer(const zchxCommon::zchxRadarCtrlDef& data);


signals:

private:
    static  MainProcess* m_pInstance;
    bool    mStartFlag;
    zchxCfgMgr*                                 mCfgMgr;
    zchxRadarMgr*                               mDevMgr;
    zchxMsgServerThread*                        mMsgSvrThrd;            //数据请求响应的线程


};

#endif // MAINPROCESS_H
