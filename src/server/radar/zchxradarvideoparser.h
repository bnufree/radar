#ifndef ZCHXRADARVIDEOPARSER_H
#define ZCHXRADARVIDEOPARSER_H

#include <QObject>
#include <QThread>
#include <QPointF>
#include <QPolygonF>
#include <QList>
#include <QPixmap>
#include <QTimer>
#include <QGeoCoordinate>
#include "zchxradarcommon.h"
#include "zchxdatadef.h"
#include "zchxradaroutputdatamgr.h"



#define SPOKES (4096)

//自定义链表结构体
struct  RadarNode{
public:
    zchxRadarRectDef        mRect;
    bool                    mIsOk;
    QList<RadarNode*>       mNextNodes;
    RadarNode()
    {
        mIsOk = false;
    }
    RadarNode(const zchxRadarRectDef& other)
    {
        mRect.CopyFrom(other);
        mIsOk = false;
    }

};

class ZCHXRadarVideoProcessor;
class zchxRadarTargetTrack;

class zchxRadarVideoParser : public QObject
{
    Q_OBJECT
public:
    explicit zchxRadarVideoParser(zchxRadarOutputDataMgr* mgr, const zchxVideoParserSettings& setting,
                                  QObject *parent = 0);
    ~zchxRadarVideoParser();
    bool    isSameParseSetting(const zchxVideoParserSettings& setting);
    void    updateParseSetting(const zchxVideoParserSettings& setting);

signals:
public slots:
    void slotSetGpsData(double, double);//实时更新从GPS传入的经纬度坐标
    //结果发送出去
    void slotSendRectList(const zchxRadarRectMap& map);
    void slotSendRadarNodeRoute(const zchxRadarRouteNodes& list);
    void slotSendTracks(const zchxRadarSurfaceTrack&);
    void slotSendVideoImage(const QImage &videoPixmap);//接收余辉图片
    //手动指定雷达类型
    void slotSetRadarType(int type);
    void slotRecvVideoData(const QByteArray& bytes);
private:
    void processVideoData(bool rotate = true);
    void InitializeLookupData();
private:
    QThread*                        mWorkThread;  //检查目标是否另外线程处理
    zchxVideoParserSettings         mParseParam;
    double                          mScanRadius;//半径
    int                             mRadarType;//读取接入雷达型号 4G/6G
    QMap<int,RADAR_VIDEO_DATA>      mRadarVideoMap1T;//一周期回波数据key是azimuth,这里需要注意统一成中间点的角度,因为有时接收到的值有可能是奇数有时是偶数
    int                             mStartAzimuth  = -1;
    ZCHXRadarVideoProcessor*        mVideoProcessor;                  //回波处理成矩形目标点
    zchxRadarTargetTrack*           mTrackProcessor;                     //矩形目标点进行目标跟踪
    int                             mDopplerVal;
    zchxRadarOutputDataMgr*         mRadarOutMgr;
    QTime                           mCounterT;
    int                             mTermSpokeCount;
    int                             mNextSpoke;
    int                             mTermIndex;
};

#endif // ZCHXRADARVIDEOPARSER_H
