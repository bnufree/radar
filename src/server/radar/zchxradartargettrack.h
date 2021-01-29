#ifndef ZCHXRADARTARGETTRACK_H
#define ZCHXRADARTARGETTRACK_H

#include <QThread>
#include <QSharedPointer>
#include "zchxradarcommon.h"
#include <QMutex>
#include "targetnode.h"
#include "zchxdatadef.h"
#include <QThread>
#include <QThreadPool>



//将预推区域和目标进行关联
struct PredictionNode{
    zchxTargetPrediction        *mPrediction;
    TargetNode*                 mNode;
};

struct AreaNodeTable{
    QString             mType;
    QPolygonF           mArea;
    QList<TargetNode*>  mNodeList;
    zchxRadarRectDefList        mRectList;          //落入这个区域的所有目标
};

#define     TARGET_CONFIRM_NODE_COUNT         5


class zchxRadarTargetTrack : public QThread
{
    Q_OBJECT
public:
    enum        TargetTrackModel{
        Model_None = 0,
        Model_Cross,
        Model_Metting,
        Model_Overtake,
    };

    explicit    zchxRadarTargetTrack(const zchxVideoParserSettings& setting, QObject *parent = 0);
    ~zchxRadarTargetTrack();
public slots:
    void        appendTrackTask(const zchxRadarRectDefList& task);
    void        setOver(bool sts) {mIsOver = sts;}
    void        updateVideoCounter(int index);
    void        updateRangeFactor(double factor) {mRangeFactor = factor;}

protected:
    void     run();
    void     addDelNodeReason(quint32 time, double lat, double lon, const QString& reason, int track);
public:

    double      calcDis(const zchxRadarRectDef& p1, const zchxRadarRectDef& p2);
    double      calcDis(zchxRadarRectDef* p1, zchxRadarRectDef* p2) {return calcDis(*p1, *p2);}
    double      calcDis(const zchxRadarRectDef& p1, zchxRadarRectDef* p2) {return calcDis(p1, *p2);}
    double      calcDis(zchxRadarRectDef* p1, const zchxRadarRectDef& p2) {return calcDis(*p1, p2);}
    void        determineNodeStatus(TargetNode* node, bool term_check);
    bool        calcNodeMostRelativeRect(TargetNode* node,  const zchxRadarRectDefList& list);
    void        CounterPossibleChildRect(QList<AreaNodeTable>& areaTableList, QMap<TargetNode*, zchxRadarRectDefList>& result ,const zchxRadarRectDefList& rects);


    void        process(const zchxRadarRectDefList& task);

    void        processWithPossibleRoute(const zchxRadarRectDefList& task);
    void        appendUserDefObj(const zchxCommon::UserSpecifiedObj& obj);
    void        removeUserDefObj(const zchxCommon::UserSpecifiedObj& obj);
    void        updateParseSetting(const zchxVideoParserSettings& setting);

    void        outputTargets();

    int         getCurrentNodeNum();
    bool        getTask(zchxRadarRectDefList& task);
    void        mergeRectTargetInDistance(zchxRadarRectDefList &temp_list, int target_merge_distance);
    Latlon      getMergeTargetLL(const zchxRadarRectDefList &list);
    void        changeTargetPixCenter(const QPoint &center, zchxRadarRectDef *cur);
    void        changeTargetLL(const Latlon &ll, zchxRadarRectDef &cur);
    bool        isDirectionChange(double src, double target);
    void        dumpTargetDistance(const QString &tag, double merge_dis);
    void        checkTargetRectAfterUpdate(double merge_dis);
    bool        isRectAreaContainsPoint(const zchxRadarRectDef& rect, double lat, double lon);
    bool        preCheckTargetValid(TargetNode* src, const zchxRadarRectDef& target);
    double      calNodeMoveDis(TargetNode* node);
    bool        isSamePoint(TargetNode* src, const zchxRadarRectDef& target);


signals:
    void        signalSendTracks(const zchxRadarSurfaceTrack& track);
    void        signalSendRectData(const zchxRadarRectMap& map);
    void        signalSendRoutePath(const zchxRadarRouteNodes& list);
    void        signalSendDelNodeLog(const QByteArray& log);
public slots:
    void        updateTrackPointWithNode(zchxRadarSurfaceTrack& list, TargetNode* node);

private:
    QList<zchxRadarRectDefList>       mTaskList;
    QMutex                      mTaskMutex;


    zchxRadarRectMap            mRadarRectMap;//用于发送的回波矩形MAP
    int                         mRectNum;
    int                         mMinNum;
    int                         mMaxNum;
    TargetNodeMap               mTargetNodeMap;                     //保存目标还未定性的点
    int                         mMaxEstCount;                       //目标的最大预推次数
    QMap<int, zchxCommon::UserSpecifiedObj>     mUserDefObj;                    //
    //目标是否预推更新
    int                         mTargetPredictionInterval;           //预推周期 比如2个周期更新一次
    zchxVideoParserSettings     mSettings;

    bool                        mIsOver;
    quint32                     mLastVideoDataTime;
    double                      mMaxSpeed;
    QThreadPool                 *mThreadPool;
    QMutex                      mVideoCounterMutex;
    QMap<int, int>              mVideoUsedCounter;
    double                      mRangeFactor;
    zchxCommon::zchxDelNodeLogList                 mDelNodeReasonList;

};

#endif // ZCHXRADARTARGETTRACK_H
