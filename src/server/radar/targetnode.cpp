#include "targetnode.h"
#include "zchxradarcommon.h"
#include <QSharedPointer>
#include <QDebug>

#define         FALSE_ALARM_INDEX_SIZE   10
#define         FALSE_ALARM_CONTINUE_EMPTY  5
#define         FALSE_ALARM_COUNTER_PERCENT  0.5

extern          bool  debug_output;
TargetNode::TargetNode()
{
    mSerialNum = 0;
    mStatus = Node_UnDef;
    mDefRect = 0;
    mUpdateTime = QDateTime::currentDateTime().toTime_t();
    mLastLostTime = 0;
    mVideoTerm = 0;
    mLastPrecitonTerm = 1;
    mChild = 0;
}
TargetNode::TargetNode(const zchxRadarRectDef& other, QSharedPointer<TargetNode> parentNode)
{
    mDefRect = new zchxRadarRectDef(other);
    mUpdateTime = mDefRect->mSrcRect.updatetime();
    mParent = parentNode;
    mSerialNum = 0;
    mStatus = Node_UnDef;
    mLastPrecitonTerm = 0;
    mChild = 0;
    mLastLostTime = 0;
    if(mParent)
    {
        mStatus = mParent->mStatus;
        mSerialNum = mParent->mSerialNum;
        mParent->mChild = this;
        mLastPrecitonTerm = mParent->mLastPrecitonTerm;
        mLastLostTime = mParent->mLastLostTime;
        if(debug_output)    qDebug()<<"new child add to parent:"<<mSerialNum<<mParent->mDefRect->mCenter<<" child:"<<mDefRect->mCenter;
    }
    mVideoTerm = other.mSrcRect.videocycleindex();

}

void TargetNode::setStatus(NodeStatus sts)
{
    mStatus = sts;
}

TargetNode::~TargetNode()
{
//    qDebug()<<"delete me:"<<this<<" reset parent:"<<mParent.data();
    if(mParent) mParent.reset();
    if(mDefRect) delete mDefRect;
}


double TargetNode::getReferenceSog(bool average)
{
    if(!mDefRect) return 0.0;
    double sum = mDefRect->mSrcRect.sogms();
    if(!average) return sum;
    int num = 1;
    TargetNode* pre = mParent.data();
    while (pre) {
        if(pre->mDefRect && pre->mParent)  //根节点没有速度  暂且不考虑
        {
            sum += pre->mDefRect->mSrcRect.sogms();
            num++;
        }
        if(num == 5) break;
        pre = pre->mParent.data();
    }

    return sum / num;
}

Latlon TargetNode::getReferencePoint()
{
    Latlon ll;
    if(!mDefRect) return ll;
    ll.lon = mDefRect->mSrcRect.center().longitude();
    ll.lat = mDefRect->mSrcRect.center().latitude();
    int num = 1;
    TargetNode* pre = mParent.data();
    while (pre) {
        if(pre->mDefRect)
        {
            ll.lon += pre->mDefRect->mSrcRect.center().longitude();
            ll.lat += pre->mDefRect->mSrcRect.center().latitude();
            num++;
        }        
        if(num == 5) break;
        pre = pre->mParent.data();
    }

    ll.lon = ll.lon / num;
    ll.lat = ll.lat / num;
    return ll;
}

void TargetNode::updateSerialNum(int num)
{
    if(mSerialNum != num) mSerialNum = num;
}


TargetNode* TargetNode::topNode()
{
    if(!mParent) return this;
    TargetNode *parent = mParent.data();
    while (parent) {
        if(parent->mParent)
        {
            parent = parent->mParent.data();
        } else
        {
            break;
        }
    }

    return parent;
}

QList<TargetNode*> TargetNode::path()
{
    QList<TargetNode*>  list;
    TargetNode *node =  this;
    while (node) {
        list.append(node);
        if(node->mParent)
        {
            node = node->mParent.data();
        } else
        {
            break;
        }
    }

    return list;
}


bool  TargetNode::isTopNode() const
{
    return mParent == 0;
}


QList<uint>  TargetNode::getVideoTermIndexList()
{
    QList<uint> result;
    result.append(mVideoTerm);
    TargetNode* parent = mParent.data();
    while (parent) {
        result.append(parent->mVideoTerm);
        parent = parent->mParent.data();
    }
    return result;
}


bool TargetNode::isFalseAlarm() const
{
    if(mStatus == Node_Moving) return false;

    //从现在开始进行遍历，如果预推个数满足要求，如果预推连续数满足要求，则定位为虚警
    const TargetNode* node = this;
    int empty_num = 0, continue_num = 0, total_num = 0;
    while (node) {
        total_num ++;
        if(node->mDefRect->mSrcRect.realdata())
        {
            continue_num = 0;
        } else
        {
            empty_num++;
            continue_num++;
        }
        if(continue_num == FALSE_ALARM_CONTINUE_EMPTY)
        {
            return true;
        }
        if(empty_num >= int(ceil(FALSE_ALARM_INDEX_SIZE * FALSE_ALARM_COUNTER_PERCENT)))
        {
            return true;
        }
        if(total_num == FALSE_ALARM_INDEX_SIZE)  break;

        node = node->mParent.data();
    }

    return false;
}

