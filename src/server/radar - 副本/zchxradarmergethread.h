#ifndef ZCHXRADARMERGETHREAD_H
#define ZCHXRADARMERGETHREAD_H

#include <QThread>
#include "zchxradarcommon.h"
#include <QMutex>

typedef QMap<int, zchxRadarSurfaceTrack>  zchxTrackMergeTask;


class zchxRadarMergeThread : public QThread
{
    Q_OBJECT
public:
    explicit zchxRadarMergeThread(QObject *parent = 0);
    void     appendTask(const zchxTrackMergeTask& task);
    bool     getTask(zchxTrackMergeTask& task);
    void     setOver() {mIsOver = true;}
private:
    void     merge(zchxRadarSurfaceTrack& result, zchxRadarSurfaceTrack& source);
    bool     same(const zchxTrackPoint& p1, const zchxTrackPoint& p2);
protected:
    void run();
signals:
public slots:
private:
    QList<zchxTrackMergeTask>   mTaskList;
    QMutex                      mMutex;
    bool                        mIsOver;
};

#endif // ZCHXRADARMERGETHREAD_H
