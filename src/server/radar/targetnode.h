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

class LeastSquare
{
public :
    LeastSquare(const QList<double>& x, const QList<double>& y)
    {
        double t1=0, t2=0, t3=0, t4=0;
        for(int i=0; i<x.size(); ++i)
        {
            t1 += x[i]*x[i];
            t2 += x[i];
            t3 += x[i]*y[i];
            t4 += y[i];
        }
        a = (t3*x.size() - t2*t4) / (t1*x.size() - t2*t2);
        b = (t1*t4 - t2*t3) / (t1*x.size() - t2*t2);
    }

    LeastSquare(const QList<QPointF> & pnts)
    {
        double t1=0, t2=0, t3=0, t4=0;
        for(int i=0; i<pnts.size(); ++i)
        {
            double  x = pnts[i].x();
            double  y = pnts[i].y();
            t1 += x * x;
            t2 += x;
            t3 += x*y;
            t4 += y;
        }
        a = (t3*pnts.size() - t2*t4) / (t1*pnts.size() - t2*t2);
        b = (t1*t4 - t2*t3) / (t1*pnts.size() - t2*t2);
    }

    double getY(double x) {return a*x+b;}

private:
    double a;
    double b;
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
    uint                                        mLastPrecitonTerm;
    TargetNode*                                 mChild;


    TargetNode();
    TargetNode(const zchxRadarRectDef& other, QSharedPointer<TargetNode> parentNode = QSharedPointer<TargetNode>());
    ~TargetNode();


    void setStatus(NodeStatus sts);
    double getReferenceSog(bool average = true);
    Latlon getReferencePoint();
    void updateSerialNum(int num);
    TargetNode* topNode();
    QList<TargetNode*> path();
    bool  isTopNode() const;
    QList<uint>  getVideoTermIndexList();
    bool  isFalseAlarm() const;

};

typedef QMap<int, QSharedPointer<TargetNode> >      TargetNodeMap;


#endif // TARGETNODE_H
