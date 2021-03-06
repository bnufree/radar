﻿#ifndef ZCHXDATAMGRFACTORY_H
#define ZCHXDATAMGRFACTORY_H

#include "zchxdatamanagers.h"

namespace qt {

//#define         ZCHX_DATA_FACTORY           zchxDataMgrFactory::instance()
class   zchxMapWidget;

class zchxDataMgrFactory
{
public:
    explicit zchxDataMgrFactory();
    ~zchxDataMgrFactory();
//    static zchxDataMgrFactory *instance();
    void setDisplayWidget(zchxMapWidget* w);

    void  createManager(int type);
    QList<std::shared_ptr<zchxEcdisDataMgr>>    getManagers() const;
    std::shared_ptr<zchxEcdisDataMgr> getManager(int type);


    bool removeDataMgr(std::shared_ptr<zchxEcdisDataMgr> mgr);
    bool appendDataMgr(std::shared_ptr<zchxEcdisDataMgr> mgr);
    //具体各个数据类型获取
    zchxAisDataMgr* getAisDataMgr();
    zchxAisSiteDataMgr* getAisSiteMgr();
    zchxAidtoNavigationDataMgr* getAidtoNavigationMgr();
    zchxRadarDataMgr* getRadarDataMgr();
    zchxRadarSiteDataMgr* getRadarSiteMgr();
    zchxRodDataMgr* getRodDataMgr();
    zchxCameraDataMgr* getCameraDataMgr();
    zchxCameraViewDataMgr* getCameraViewMgr();
    zchxVideoTargetDataMgr* getVideoDataMgr();
    zchxPastrolStationDataMgr* getPastrolStationMgr();
    zchxWarningZoneDataMgr* getWarningZoneMgr();
    zchxCoastDataMgr*   getCoastMgr();
    zchxSeabedPipLineDataMgr* getSeabedPiplineMgr();
    zchxChannelDataMgr* getChannelMgr();
    zchxStructureDataMgr*   getStructureMgr();
    zchxAreaNetDataMgr* getAreanetMgr();
    zchxMooringDataMgr* getMooringMgr();
    zchxCardMouthDataMgr*   getCardmouthMgr();
    zchxStatistcLineDataMgr*   getStatistcLineMgr();
    zchxLocalMarkDataMgr*   getLocalmarkMgr();
    zchxDangerousDataMgr*   getDangerousMgr();
    zchxIslandLineDataMgr*  getIslandlineMgr();
    zchxNetGridDataMgr*  getNetGridMgr();
    zchxShipAlarmAscendDataMgr* getShipAlarmAscendMgr();
    zchxRadarRectMgr*          getRadarRectMgr();
    zchxRadarVideoMgr*          getRadarVideoMgr();
    zchxRadarFeatureZoneDataMgr* getRadarFeatureZoneMgr();
    zchxNaviMarkDataMgr* getNaviMarkDataMgr();
    zchxAisChartDataMgr* getAisChartMgr();

    zchxPointDataMgr* getPointMgr();
    zchxLineDataMgr* getLineMgr();
    zchxPolygonDataMgr* getPolygonMgr();
    zchxRadarNodePathDataMgr* getRadarRoutePathMgr();
    zchxRadarNodeLogDataMgr*  getRadarNodeLogMgr();

private:
    static zchxDataMgrFactory     *minstance;

    class MGarbage // 它的唯一工作就是在析构函数中删除CSingleton的实例
    {
    public:
        ~MGarbage()
        {
            if (zchxDataMgrFactory::minstance) delete zchxDataMgrFactory::minstance;
        }
    };
    static MGarbage Garbage; // 定义一个静态成员，在程序结束时，系统会调用它的析构函数

    zchxMapWidget*      mWidget;
    QMap<int, std::shared_ptr<zchxEcdisDataMgr>>  mMgrList;

};
}

#endif // ZCHXDATAMGRFACTORY_H
