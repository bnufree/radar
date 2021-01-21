#include "zchxcfgmgr.h"
#include <QCoreApplication>
#include <QFile>
#include "zchxcommonutils.h"
#include <QDebug>
#include <QDir>
#include "dataout/zchxdataoutputservermgr.h"

bool        debug_output = false;

zchxCfgMgr::zchxCfgMgr(const QString& fileName, QObject *parent) :
    QObject(parent),
    mFileName(fileName),
    mCfg(0)
{
    qRegisterMetaType<zchxCommon::zchxPublishSettingsList>("const zchxCommon::zchxPublishSettingsList&");
    qRegisterMetaType<zchxCommon::zchxRadarDeviceList>("const zchxCommon::zchxRadarDeviceList&");

    //读取配置文件
    if(mFileName.isEmpty())
    {
        QString path =  QString("%1/etc").arg(QCoreApplication::applicationDirPath());
        QDir dir(path);
        if(!dir.exists())
        {
            dir.mkpath(path);
        }
        mFileName = QString("%1/config.json").arg(path);
    }
    QFile file(mFileName);
    if(!file.exists())
    {
        if(file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            zchxCommon::zchxRadarServerCfg cfg;
            cfg.app = QCoreApplication::applicationName();
            cfg.date = QDate::currentDate().toString("yyyy-MM-dd");
            cfg.server = zchxCommon::zchxSocket(zchxCommonUtils::getAllIpv4List().first(), 6666);
            cfg.version = "1.0.0";
            cfg.publish_list.append(zchxCommon::zchxPublishSetting(zchxCommon::THREAD_DATA_RADAR_VIDEO, "RadarVideo", 5151, QString::fromUtf8("雷达回波")));
            cfg.publish_list.append(zchxCommon::zchxPublishSetting(zchxCommon::THREAD_DATA_RADAR_TRACK, "RadarTrack", 5152, QString::fromUtf8("雷达目标")));
//            cfg.publish_list.append(zchxCommon::zchxPublishSetting(zchxCommon::THREAD_DATA_FILTER, "RadarFilter", 5153, QString::fromUtf8("屏蔽区域")));
            cfg.publish_list.append(zchxCommon::zchxPublishSetting(zchxCommon::THREAD_DATA_RADAR_REPORT, "RadarReport", 5154, QString::fromUtf8("雷达状态")));
//            cfg.publish_list.append(zchxCommon::zchxPublishSetting(zchxCommon::THREAD_DATA_AIS, "Ais", 5155, QString::fromUtf8("船舶AIS")));
            cfg.publish_list.append(zchxCommon::zchxPublishSetting(zchxCommon::THREAD_DATA_SVRCFG, "SvrCfg", 5156, QString::fromUtf8("配置信息")));

            QJsonDocument doc(cfg.toJson().toObject());
            file.write(doc.toJson());
            file.close();
        }
    }

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug()<<"open cfg file failed."<<mFileName;
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if(!doc.isObject())
    {
        qDebug()<<"cfg file is not a json format file";
        return;
    }
    mCfg = new zchxCommon::zchxRadarServerCfg(doc.object());
    debug_output = mCfg->debug_output;
    QString local_ip = zchxCommonUtils::getAllIpv4List().first();
    if(mCfg->server.ip != local_ip)
    {
        mCfg->server.ip = local_ip;
        save();
    }
}

zchxCfgMgr::~zchxCfgMgr()
{
    save();
    if(mCfg)
    {
        delete mCfg;
    }
}

bool zchxCfgMgr::save()
{
    if(!mCfg) return false;
    QFile file(mFileName);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    file.write(QJsonDocument(mCfg->toJson().toObject()).toJson());
    file.close();
    return true;
}

void zchxCfgMgr::updateBaseSetting(zchxCommon::zchxRadarDeviceBaseSetting &base)
{
    QList<int> exist_id_list;
    bool found = false;
    for(int i=0; i<mCfg->device_list.size(); i++)
    {
        exist_id_list.append(mCfg->device_list[i].base.id);
        if(mCfg->device_list[i].base.id == base.id)
        {
            found = true;
            mCfg->device_list[i].base = base;
            break;
        }
    }
    if(!found)
    {
        int id = 1;
        while (exist_id_list.contains(id)) {
            id++;
        }
        base.id = id;
        zchxCommon::zchxRadarDevice dev;
        dev.base = base;
        mCfg->device_list.append(dev);
    }
}

void zchxCfgMgr::updateChannelSetting(zchxCommon::zchxRadarDevice& dev)
{
    for(int i=0; i<mCfg->device_list.size(); i++)
    {
        zchxCommon::zchxRadarDevice &exist_dev = mCfg->device_list[i];
        if(exist_dev.base.id == dev.base.id)
        {
            if(exist_dev.channel_list.size() <= dev.channel_list.size())
            {
                //更新设备的通道
                zchxCommon::zchxRadarChannelList new_channl_list = dev.channel_list;
                foreach (zchxCommon::zchxRadarChannel channel , new_channl_list) {
                    QList<int> exist_id_list;
                    bool found = false;
                    for(int k=0; k<exist_dev.channel_list.size(); k++)
                    {
                        exist_id_list.append(exist_dev.channel_list[k].id);
                        if(exist_dev.channel_list[k].id == channel.id)
                        {
                            found = true;
                            exist_dev.channel_list[k] = channel;
                            break;
                        }
                    }
                    if(!found)
                    {
                        int id = 1;
                        while (exist_id_list.contains(id)) {
                            id++;
                        }
                        channel.id = id;
                        exist_dev.channel_list.append(channel);
                    }
                }
            } else
            {
                exist_dev.channel_list = dev.channel_list;
            }

            break;
        }
    }
}

void zchxCfgMgr::updateDevRecvIP(const QString& ip, int id)
{
    for(int i=0; i<mCfg->device_list.size(); i++)
    {
        if(mCfg->device_list[i].base.id == id)
        {
            mCfg->device_list[i].local_connect_ip = ip;
            break;
        }
    }
    save();
}

void zchxCfgMgr::updateChannelHeadData(int radar, int channel, int head)
{
    for(int i=0; i<mCfg->device_list.size(); i++)
    {
        if(mCfg->device_list[i].base.id == radar)
        {
            mCfg->device_list[i].parse_param.head = head;
            break;
        }
    }
    save();
}

void zchxCfgMgr::updateCfg(int cmd, const QJsonValue& val)
{
    if(!mCfg) return;
    if(cmd == zchxCommon::Msg_Update_PublishSetting)
    {
        mCfg->publish_list.clear();
        mCfg->publish_list = zchxCommon::zchxPublishSettingsList(val.toArray());
        emit signalNewPublishSettings(mCfg->publish_list);
        //通知广播出去
        QJsonObject obj;
        obj.insert(JSON_CMD, zchxCommon::SVR_UPDATE_PUBLISH_SETTING);
        obj.insert(JSON_VAL, mCfg->publish_list.toJson());
        PUBLISH_INS->signalSendPublishData(zchxCommon::THREAD_DATA_SVRCFG, QJsonDocument(obj).toJson());
    } else
    {
        if(cmd == zchxCommon::Msg_Update_RadarBaseSetting)
        {
            zchxCommon::zchxRadarDevice dev(val.toObject());
            zchxCommon::zchxRadarDeviceBaseSetting base = dev.base;
            updateBaseSetting(base);
        } else if(cmd == zchxCommon::Msg_Delete_Radar)
        {
            int id = val.toInt();
            for(int i=0; i<mCfg->device_list.size(); i++)
            {
                if(mCfg->device_list[i].base.id == id)
                {
                    mCfg->device_list.removeAt(i);
                    break;
                }
            }

        } else if(cmd == zchxCommon::Msg_Update_ParseSetting)
        {
            zchxCommon::zchxRadarDevice dev(val.toObject());
            for(int i=0; i<mCfg->device_list.size(); i++)
            {
                if(mCfg->device_list[i].base.id == dev.base.id)
                {
                    mCfg->device_list[i].parse_param = dev.parse_param;
                    break;
                }
            }
        } else if(cmd == zchxCommon::Msg_Update_ChannelSetting)
        {
            zchxCommon::zchxRadarDevice dev(val.toObject());
            updateChannelSetting(dev);
        } else if(cmd == zchxCommon::Msg_Update_BaseAndChannel)
        {
            zchxCommon::zchxRadarDevice dev(val.toObject());
            updateBaseSetting(dev.base);
            updateChannelSetting(dev);
        }

        emit signalNewRadarSettings(mCfg->device_list);

        QJsonObject obj;
        obj.insert(JSON_CMD, zchxCommon::SVR_UPDATE_RADAR_DEV);
        obj.insert(JSON_VAL, mCfg->device_list.toJson());
        PUBLISH_INS->signalSendPublishData(zchxCommon::THREAD_DATA_SVRCFG, QJsonDocument(obj).toJson());
    }
    save();

}

void zchxCfgMgr::updateSocketStatus(const zchxCommon::zchxPortStatus &status)
{
    updateSocketStatus(zchxCommon::zchxPortStatusList(status));
}

void zchxCfgMgr::updateSocketStatus(const zchxCommon::zchxPortStatusList &list)
{
    foreach (zchxCommon::zchxPortStatus sts, list) {
        if(sts.port == mCfg->server.port){
            mCfg->server.status = sts.sts;
        } else
        {
            for(int i=0; i<mCfg->publish_list.size();i++)
            {
                zchxCommon::zchxPublishSetting& setting = mCfg->publish_list[i];
                if(setting.port == sts.port)
                {
                    setting.status = sts.sts;
                }
            }
        }
    }
    //通知广播出去
    QJsonObject obj;
    obj.insert(JSON_CMD, zchxCommon::SVR_UPDATE_PUBLISH_PORT_STATUS);
    obj.insert(JSON_VAL, mCfg->publish_list.toJson());
    PUBLISH_INS->signalSendPublishData(zchxCommon::THREAD_DATA_SVRCFG, QJsonDocument(obj).toJson());
}
