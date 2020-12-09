#include "zchxradarmergethread.h"
#include "zchxradarcommon.h"
#include "dataout/zchxdataoutputservermgr.h"
#include <QDebug>
#include <QGeoCoordinate>

zchxRadarMergeThread::zchxRadarMergeThread(QObject *parent) : QThread(parent)
{
    mIsOver = false;
}

void zchxRadarMergeThread::appendTask(const zchxTrackMergeTask &task)
{
    QMutexLocker locker(&mMutex);
    mTaskList.clear();
    mTaskList.append(task);
}

bool zchxRadarMergeThread::getTask(zchxTrackMergeTask &task)
{
    QMutexLocker locker(&mMutex);
    if(mTaskList.size() == 0) return false;
    task = mTaskList.takeFirst();
    return true;
}

void zchxRadarMergeThread::run()
{
    while (!mIsOver) {
        zchxTrackMergeTask task;
        if(getTask(task))
        {
            if(task.size() > 0)
            {
                //开始合并
                QList<int> keys = task.keys();
                zchxRadarSurfaceTrack resultList = task[keys[0]];
                for(int i=1; i<keys.size(); i++)
                {
                    merge(resultList, task[keys[i]]);
                }
                zchxRadarSurfaceTrack* track = new zchxRadarSurfaceTrack(resultList);
                qDebug()<<"send track point size:"<<track->trackpoints_size();
                emit PUBLISH_INS->signalSendPublishData(zchxCommon::THREAD_DATA_RADAR_TRACK, zchxRadarUtils::protoBufMsg2ByteArray(track));
                delete track;
            }
        }

        QThread::msleep(500);
    }
}

bool zchxRadarMergeThread::same(const zchxTrackPoint &p1, const zchxTrackPoint &p2)
{
    //检查两者的角度
    if(fabs(p1.current().cog() - p2.current().cog()) < 180)
    {
        //检查两者的距离
        QGeoCoordinate src(p1.current().center().latitude(), p1.current().center().longitude());
        double dis = src.distanceTo(QGeoCoordinate(p2.current().center().latitude(), p2.current().center().longitude()));
        if(dis <= 10)
        {
            return true;
        }
    }

    return false;

}

void zchxRadarMergeThread::merge(zchxRadarSurfaceTrack &result, zchxRadarSurfaceTrack &srcList)
{
    QList<zchxTrackPoint> addlist;
    for(int k=0; k<srcList.trackpoints_size(); k++)
    {
        zchxTrackPoint src = srcList.trackpoints(k);
        bool found = false;
        for(int i=0; i<result.trackpoints_size(); i++)
        {
            zchxTrackPoint dest = result.trackpoints(i);
            if(same(dest, src))
            {
                found = true;
                break;
            }
        }
        if(!found) addlist.append(src);
    }

    foreach (zchxTrackPoint pnt, addlist) {
        zchxTrackPoint* add = result.add_trackpoints();
        add->CopyFrom(pnt);
    }

}
