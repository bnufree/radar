#include "mainprocess.h"
#include <QDebug>
#include <QDateTime>
#include "zchxcfgmgr.h"
#include "radar/zchxradarmgr.h"
#include "dataout/zchxdataoutputservermgr.h"

MainProcess* MainProcess::m_pInstance = 0;
bool output_log_std = false;

MainProcess::MainProcess(const QString& fileName, QObject *parent) :
    QObject(parent),
    mCfgMgr(new zchxCfgMgr(fileName)),
    mMsgSvrThrd(0),
    mDevMgr(0)
{
}

MainProcess::~MainProcess()
{
    if(mCfgMgr) delete mCfgMgr;
    if(mDevMgr) delete mDevMgr;
    if(mMsgSvrThrd) delete mMsgSvrThrd;
    delete PUBLISH_INS;
}

MainProcess* MainProcess::instance()
{
    if(!m_pInstance) m_pInstance = new MainProcess;
    return m_pInstance;
}


bool MainProcess::isDataServerOk()
{
//    int server_port = mDataTypeMap[THREAD_DATA_SERVER];
//    zchxDataThread* thread = getThread(server_port);
//    if(thread) return thread->isOK();
    return false;
}


bool MainProcess::start()
{
    //获取配置文件
    qDebug()<<"start server 20210115. please wait for initlization from file:"<<mCfgMgr->getCfgFile();
    if(mCfgMgr->isError())
    {
        qDebug()<<"error occured while reading config file, please recheck config file...";
        return false;
    }
    //开启数据数据服务
    connect(PUBLISH_INS, SIGNAL(signalSendPortStartStatus(zchxCommon::zchxPortStatusList)), this, SLOT(slotRecvPublishPortStartStatus(zchxCommon::zchxPortStatusList)));
    mMsgSvrThrd = new zchxMsgServerThread(PUBLISH_INS->getCtx(), mCfgMgr->getMainDataPort());
    mMsgSvrThrd->setCfgMgr(mCfgMgr);
    connect(mMsgSvrThrd, SIGNAL(signalPortStatus(zchxCommon::zchxPortStatus)), this, SLOT(slotRecvMsgPortStatus(zchxCommon::zchxPortStatus)));
    connect(mMsgSvrThrd, SIGNAL(signalSendCtrlFromClient(zchxCommon::zchxRadarCtrlDef)),
            this, SLOT(slotRecvCtrlFromServer(zchxCommon::zchxRadarCtrlDef)));
    connect(mMsgSvrThrd, SIGNAL(signalSendEditFilterAreas(int,zchxCommon::zchxfilterAreaList)),
            this, SLOT(updateFilterAreaMsg(int,zchxCommon::zchxfilterAreaList)));
    mMsgSvrThrd->start();
    connect(mCfgMgr, SIGNAL(signalNewPublishSettings(zchxCommon::zchxPublishSettingsList)),
            PUBLISH_INS, SIGNAL(signalInitPublish(zchxCommon::zchxPublishSettingsList)));
//    emit mCfgMgr->signalNewPublishSettings(mCfgMgr->getPubSettings());
//    mPublishMgr->initFromCfg(mCfgMgr->getPubSettings());

    //开启雷达获取雷达数据
    mDevMgr = new zchxRadarMgr(mCfgMgr->config()->filter_enabled);
    connect(mCfgMgr, SIGNAL(signalNewRadarSettings(zchxCommon::zchxRadarDeviceList)),
            mDevMgr, SLOT(updateDeviceList(zchxCommon::zchxRadarDeviceList)));
    connect(mDevMgr, SIGNAL(signalSendRecvIP(QString,int)),  mCfgMgr, SLOT(updateDevRecvIP(QString,int)));
    connect(mDevMgr, SIGNAL(signalSendHeadChangedData(int,int,int)), mCfgMgr, SLOT(updateChannelHeadData(int,int,int)));
//    emit mCfgMgr->signalNewRadarSettings(mCfgMgr->getDevList());
    return true;
}

void MainProcess::slotRecvPublishPortStartStatus(const zchxCommon::zchxPortStatusList &list)
{
    qDebug()<<"recv publish port start status.";
    foreach (zchxCommon::zchxPortStatus data, list) {
        qDebug()<<"server port start with status:"<<data.port<<data.sts<<data.topic;
        if(!data.sts)
        {
            qDebug()<<"server start failed. please restart now....";
            return ;
        }
    }

    if(mCfgMgr) mCfgMgr->updateSocketStatus(list);

    //开始启动雷达
    qDebug()<<"start init radar device....";
    emit mCfgMgr->signalNewRadarSettings(mCfgMgr->getDevList());

}

void MainProcess::slotRecvMsgPortStatus(const zchxCommon::zchxPortStatus &data)
{
    qDebug()<<QString("main server thread port status. %1:%2").arg(data.port).arg(data.sts);
    if(mCfgMgr) mCfgMgr->updateSocketStatus(data);
    if(!data.sts)
    {
        qDebug()<<"server start failed. please restart now....";
        return ;
    }

    //开始启动数据传输服务
    emit mCfgMgr->signalNewPublishSettings(mCfgMgr->getPubSettings());
}

void MainProcess::updateFilterStatus(bool sts)
{
    if(mDevMgr) mDevMgr->updateDevFilterStatus(sts);
}

bool MainProcess::updateFilterAreaMsg(int cmd, const zchxCommon::zchxfilterAreaList &area)
{
    if(mDevMgr)
    {
        mDevMgr->updateFilterArea(cmd, area);
        return true;
    }
    return false;
}

void MainProcess::slotRecvCtrlFromServer(const zchxCommon::zchxRadarCtrlDef &data)
{
    if(mDevMgr)
    {
        mDevMgr->setControl(data);
    }
}
