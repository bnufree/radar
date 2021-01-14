#ifndef TARGETNODE_H
#define TARGETNODE_H

#include "zchxradarcommon.h"
#include <QSharedPointer>

enum  TARGET_DIRECTION{
    TARGET_DIRECTION_UNDEF = 0,     //点列不足的情况
    TARGET_DIRECTION_STABLE,        //目标点列方向稳定
    TARGET_DIRECTION_UNSTABLE,      //目标点列方向散乱
};


//根节点可以产生很多条分支,但是每一个分支点只有一个子节点
enum NodeStatus{
    Node_UnDef = 0,
    Node_NotMove,                   //静止
    Node_Moving,                    //移动
    Node_Lost,
    Node_GAP_ABNORMAL,
};

//目标状态确定为静止，则目标路径上就始终只有一个点
struct TargetNode
{
public:
    int                                         mSerialNum;
    NodeStatus                                  mStatus;
    uint                                         mUpdateTime;        //目标最新的更新时间
    zchxRadarRectDef                            *mDefRect;
    QSharedPointer<TargetNode>                  mParent;             //父亲
    uint                                        mLastLostTime;      //目标丢失的时间
    zchxRadarRectDefList                        mRelativeRectList;
    uint                                        mVideoTerm;
    int                                         mLevel;
    TargetNode*                                 mChild;


    TargetNode();
    TargetNode(const zchxRadarRectDef& other, QSharedPointer<TargetNode> parentNode = QSharedPointer<TargetNode>());
    ~TargetNode();


    void setStatus(NodeStatus sts);
    double getReferenceSog(bool average = true);
    Latlon getReferencePoint();
    void updateSerialNum(int num);
    TargetNode* topNode();
    int     getDepth();
    bool  isTopNode() const;
    QList<uint>  getVideoTermIndexList();
    bool  isFalseAlarm() const;

};

typedef QMap<int, QSharedPointer<TargetNode> >      TargetNodeMap;


#endif // TARGETNODE_H
