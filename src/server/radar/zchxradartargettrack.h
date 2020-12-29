#ifndef ZCHXRADARTARGETTRACK_H
#define ZCHXRADARTARGETTRACK_H

#include <QThread>
#include <QSharedPointer>
#include "zchxradarcommon.h"
#include <QMutex>
#include "targetnode.h"
#include "zchxdatadef.h"
#include <QThread>



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

protected:
    void     run();
private:
    void        CounterPossibleChildRect(QList<AreaNodeTable>& areaTableList, QMap<TargetNode*, zchxRadarRectDefList>& result ,const zchxRadarRectDefList& rects);
    void        UpdateNodeWithPossibleTarget(QMap<TargetNode*, zchxRadarRectDefList>& counter, QList<int>& used_rect_list);
    void        UpdateNodeWithExistVideoRect(QList<TargetNode*> list, QList<int>& used_index_list, const zchxRadarRectDefList& rects);
    void        determineTargetStatus();
    void        targetRecheck(int term);
    void        PredictionMoveNode(int term, quint32 now);

    void        process(const zchxRadarRectDefList& task);
    void        processWithPossibleRoute(const zchxRadarRectDefList& task);
    void        appendUserDefObj(const zchxCommon::UserSpecifiedObj& obj);
    void        removeUserDefObj(const zchxCommon::UserSpecifiedObj& obj);
    void        updateParseSetting(const zchxVideoParserSettings& setting);

    QList<AreaNodeTable>  calculateTargetTrackMode(double max_speed, quint32 now, double scan_time);
    NodeStatus        checkRoutePathSts(QList<TargetNode*>& newRoueNodeList, const QList<TargetNode*>& path);
    void        splitAllRoutesIntoTargets(TargetNode* node, TargetNode* routeNode);
    void        deleteExpiredNode();
    void        outputTargets();
    void        outputRoutePath();
    void        updateTrackPointWithNode(zchxRadarSurfaceTrack& list, TargetNode* node, int* silent_num = 0);
    void        updateRectMapWithNode(zchxRadarRectMap& map, TargetNode* node);

    int         getCurrentNodeNum();
    void        appendNode(TargetNode* node, int source);
    bool        getTask(zchxRadarRectDefList& task);
    void        mergeRectTargetInDistance(zchxRadarRectDefList &temp_list, int target_merge_distance);
    Latlon      getMergeTargetLL(const zchxRadarRectDefList &list);
    void        changeTargetLL(const Latlon &ll, zchxRadarRectDef &cur);
    bool        isDirectionChange(double src, double target);
    void        dumpTargetDistance(const QString &tag, double merge_dis);
    void        checkTargetRectAfterUpdate(double merge_dis);
    bool        isRectAreaContainsPoint(const zchxRadarRectDef& rect, double lat, double lon);


signals:
    void        signalSendTracks(const zchxRadarSurfaceTrack& track);
    void        signalSendRectData(const zchxRadarRectMap& map);
    void        signalSendRoutePath(const zchxRadarRouteNodes& list);
public slots:

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

};

#endif // ZCHXRADARTARGETTRACK_H
