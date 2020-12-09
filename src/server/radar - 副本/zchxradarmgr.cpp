#include "zchxradarmgr.h"
#include <QCoreApplication>
#include <QDir>
#include <QTimer>
#include "dataout/zchxdataoutputservermgr.h"

zchxRadarMgr::zchxRadarMgr(bool filter, QObject *parent) :
    QObject(parent),
    mFilterEnabled(filter)
{
    QString path =  QString("%1/etc").arg(QCoreApplication::applicationDirPath());
    QDir dir(path);
    if(!dir.exists())
    {
        dir.mkpath(path);
    }
    mFilterMgr = new zchxFilterAreaFile(QString("%1/filter.json").arg(path));
    mDevList.clear();
    QTimer *filterTimer = new QTimer(this);
    filterTimer->setInterval(30000);
    connect(filterTimer, SIGNAL(timeout()), this, SLOT(slotSendFilterAreaData()));
    filterTimer->start();
}

zchxRadarMgr::~zchxRadarMgr()
{
    if(mFilterMgr)
    {
        delete mFilterMgr;
        mFilterMgr = 0;
    }
    mDevMgr.clear();
}


void zchxRadarMgr::updateFilterArea(int cmd, const zchxCommon::zchxfilterAreaList &area)
{
    if(!mFilterMgr) return;

    if(cmd == zchxCommon::Msg_Edit_FilterArea)
    {
        mFilterMgr->addArea(area);
    } else if(cmd == zchxCommon::Msg_Delete_FilterArea)
    {
        QList<int> ids;
        foreach (zchxCommon::zchxFilterArea d, area) {
            ids.append(d.id);
        }
        mFilterMgr->removeArea(ids);
    }

    slotSendFilterAreaData();

    updateDevFilterStatus(mFilterEnabled);
}

void zchxRadarMgr::setControl(const zchxCommon::zchxRadarCtrlDef& def)
{
    foreach (QSharedPointer<ZCHXRadarDevice> dev, mDevMgr)
    {
        if(dev && dev->id() == def.radar_id)
        {
            dev->setCtrl(def.ch_id, def.type, def.jsval);
        }
    }
}


void zchxRadarMgr::updateDevFilterStatus(bool sts)
{
    mFilterEnabled = sts;
    foreach (QSharedPointer<ZCHXRadarDevice> dev, mDevMgr)
    {
        if(dev)
        {
            dev->updateFilterArea(sts, mFilterMgr->getFilterAreaList());
        }
    }
}

void zchxRadarMgr::updateDeviceList(const zchxCommon::zchxRadarDeviceList &list)
{
    //检查是否存在删除雷达设备的情况。
    QList<int> old_dev_del_list = mDevMgr.keys();
    foreach (zchxCommon::zchxRadarDevice newdev, list)
    {
        if(old_dev_del_list.contains(newdev.base.id))
        {
            //雷达设备已经存在，检查通道数据是否发生了变化
            QSharedPointer<ZCHXRadarDevice> dev = mDevMgr.value(newdev.base.id);
            if(dev)
            {
                dev->updateDev(newdev);
            }
            old_dev_del_list.removeOne(newdev.base.id);
        } else
        {
            //新添加的设备
            mDevMgr[newdev.base.id] = QSharedPointer<ZCHXRadarDevice>(new ZCHXRadarDevice(newdev, mFilterEnabled, mFilterMgr->getFilterAreaList()));
        }
    }

    foreach (int key, old_dev_del_list) {
        mDevMgr.remove(key);
    }
}

void zchxRadarMgr::slotSendFilterAreaData()
{
    if(!mFilterMgr) return;
    QJsonObject obj;
    obj.insert(JSON_CMD, zchxCommon::SVR_UPDATE_RADAR_FILTER);
    obj.insert(JSON_VAL, mFilterMgr->getFilterAreaList().toJson());
    emit PUBLISH_INS->signalSendPublishData(zchxCommon::THREAD_DATA_SVRCFG, QJsonDocument(obj).toJson());
}
