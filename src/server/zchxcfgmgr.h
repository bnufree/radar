#ifndef ZCHXCFGMGR_H
#define ZCHXCFGMGR_H

#include <QObject>
#include "zchxdatadef.h"

class zchxCfgMgr : public QObject
{
    Q_OBJECT
public:    
    ~zchxCfgMgr();
    bool save();
    explicit zchxCfgMgr(const QString& fileName = QString(), QObject *parent = 0);
    zchxCommon::zchxRadarServerCfg* config() {return mCfg;}
    bool isError() const {return !mCfg;}
    QString getCfgFile() const {return mFileName;}
    int getMainDataPort() const {return mCfg->server.port;}
    zchxCommon::zchxPublishSettingsList     getPubSettings() const {return mCfg->publish_list;}
    zchxCommon::zchxRadarDeviceList        getDevList() const {return mCfg->device_list;}
    void updateCfg(int cmd, const QJsonValue& val);
    void updateSocketStatus(const zchxCommon::zchxPortStatusList& list);
    void updateSocketStatus(const zchxCommon::zchxPortStatus& status);
public slots:
    void updateDevRecvIP(const QString& ip, int id);
private:
    void updateBaseSetting(zchxCommon::zchxRadarDeviceBaseSetting& base);
    void updateChannelSetting(zchxCommon::zchxRadarDevice& dev);

signals:
    void signalNewPublishSettings(const zchxCommon::zchxPublishSettingsList& list);
    void signalNewRadarSettings(const zchxCommon::zchxRadarDeviceList& list);

private:
    QString mFileName;
    zchxCommon::zchxRadarServerCfg*        mCfg;
};

#endif // ZCHXCFGMGR_H
