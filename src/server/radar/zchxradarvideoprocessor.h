#ifndef ZCHXRADARVIDEOPROCESSOR_H
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

typedef zchxRadarVideoSourceDataList      ZCHXRadarVideoProcessorData;
class zchxRadarRectExtraction;

class ZCHXRadarVideoProcessor : public QThread
{
    Q_OBJECT
public:
    explicit ZCHXRadarVideoProcessor(const zchxVideoParserSettings& setting, QObject *parent = 0);
    ~ZCHXRadarVideoProcessor();
    void appendSrcData(const zchxRadarVideoSourceData& task);
    bool getProcessData(ZCHXRadarVideoProcessorData& task);
    void setOver(bool sts) {mIsover = sts;}

protected:
    void run();


signals:
    void signalSendParsedVideoData(const zchxRadarRectDefList& list);
    void signalSendVideoPixmap(const QImage& img);

public slots: 
    void slotSetColor(int,int,int,int,int,int);
private:
    void    process(const ZCHXRadarVideoProcessorData& task);//绘制回波
    QColor  getColor(double dValue);//通过振幅值获取对应颜色值
    bool    mergeVideoOfMultiTerms(QMap<int,RADAR_VIDEO_DATA>& result, const ZCHXRadarVideoProcessorData& task);
    bool    drawVideoImage(QPaintDevice* result, const QMap<int,RADAR_VIDEO_DATA>& video, const QColor& color = QColor());
private:
    QColor                              m_objColor1;
    QColor                              m_objColor2;
    bool                                mOutputImg;
    zchxRadarVideoSourceDataList        mTaskList;
    QMutex                              mMutex;
    zchxVideoParserSettings             mParse;
    bool                                mIsover;
    zchxRadarRectExtraction*            mTargetExt;
};

#endif // ZCHXRADARVIDEOPROCESSOR_H
