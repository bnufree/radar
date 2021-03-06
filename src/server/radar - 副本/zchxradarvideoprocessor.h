﻿#ifndef ZCHXRADARVIDEOPROCESSOR_H
#define ZCHXRADARVIDEOPROCESSOR_H

#include <QObject>
#include <QThread>
#include <QMap>
#include <QPointF>
#include <QPolygonF>
#include <QMutex>
#include <QPixmap>
#include <QColor>
#include "zchxradarcommon.h"
#include "zchxdatadef.h"

class zchxRadarRectExtraction;
class zchxRadarTargetTrack;

typedef zchxRadarVideoTaskList      ZCHXRadarVideoProcessorData;

class ZCHXRadarVideoProcessor : public QThread
{
    Q_OBJECT
public:
    explicit ZCHXRadarVideoProcessor(const zchxVideoParserSettings& setting, QObject *parent = 0);
    ~ZCHXRadarVideoProcessor();
    void appendSrcData(const zchxRadarVideoTask& task);
    bool getProcessData(ZCHXRadarVideoProcessorData& task);
    void setTracker(zchxRadarTargetTrack* track) {mTracker = track;}
    void setOver(bool sts) {mIsover = sts;}
    void updateParseSetting(const zchxVideoParserSettings& setting);

protected:
    void run();


signals:
    void signalSendRects(const zchxRadarRectDefList& list);
    void signalSendVideoPixmap(const QImage& img);

public slots: 
    void slotSetColor(int,int,int,int,int,int);
private:
    void    process(const ZCHXRadarVideoProcessorData& task);//绘制回波
    QColor  getColor(double dValue);//通过振幅值获取对应颜色值
    bool    mergeVideoOfMultiTerms(QMap<int,RADAR_VIDEO_DATA>& result, const ZCHXRadarVideoProcessorData& task);
    bool    drawOriginalVideoImage(QPaintDevice* result, const QMap<int,RADAR_VIDEO_DATA>& video);
private:
    QColor                          m_objColor1;
    QColor                          m_objColor2;
    zchxRadarRectExtraction*        mVideoExtractionWorker;
    bool                            mOutputImg;
    QList<ZCHXRadarVideoProcessorData>          mTaskList;
    QMutex                          mMutex;
    zchxRadarTargetTrack            *mTracker;
    zchxVideoParserSettings         mParse;
    bool                            mIsover;
};

#endif // ZCHXRADARVIDEOPROCESSOR_H
