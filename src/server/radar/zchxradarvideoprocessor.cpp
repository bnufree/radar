#include "zchxradarvideoprocessor.h"
//#include <QDebug>
#include <math.h>
#include <QMutex>
#include <QPixmap>
#include <QPainter>
#include <QImage>
#include <QGeoCoordinate>
#include <QCoreApplication>
#include <QtMath>
#include <QDebug>
#include "zchxradarrectextraction.h"

extern bool   debug_output;
#define     DEBUG_TRACK_INFO                if(debug_output) qDebug()

ZCHXRadarVideoProcessor::ZCHXRadarVideoProcessor(const zchxVideoParserSettings& setting, QObject *parent)
    : QThread(parent)
    , mIsover(false)
    , mParse(setting)
    , mTargetExt(new zchxRadarRectExtraction(setting))
{
    qRegisterMetaType<zchxRadarRectDefList>("const zchxRadarRectDefList&");
    m_objColor1 = QColor(6,144,36);
    m_objColor2 = QColor(103,236,231);
    setStackSize(64000000);
}


ZCHXRadarVideoProcessor::~ZCHXRadarVideoProcessor()
{
    if(mTargetExt)
    {
        delete mTargetExt; mTargetExt = 0;
    }
}


void ZCHXRadarVideoProcessor::appendSrcData(const zchxRadarVideoSourceData &task)
{
    QMutexLocker locker(&mMutex);
    mTaskList.append(task);
    DEBUG_TRACK_INFO<<"after append task size:"<<mTaskList.size()<<" overlap cnt:"<<mParse.user_video_parse.video_overlap_cnt;
}

bool ZCHXRadarVideoProcessor::getProcessData(ZCHXRadarVideoProcessorData& task)
{
    QMutexLocker locker(&mMutex);
    int size = mTaskList.size();
    if(size < mParse.user_video_parse.video_overlap_cnt) return false;
    int start_index = size - mParse.user_video_parse.video_overlap_cnt;
    task = mTaskList.mid(start_index, mParse.user_video_parse.video_overlap_cnt);
    mTaskList = mTaskList.mid(start_index + 1);
    DEBUG_TRACK_INFO<<"task left size:"<<mTaskList.size()<<" overlap cnt:"<<mParse.user_video_parse.video_overlap_cnt;
    return true;
}

void ZCHXRadarVideoProcessor::run()
{
    while (!mIsover) {
        //获取当前的任务
        ZCHXRadarVideoProcessorData task;
        if(!getProcessData(task))
        {
            msleep(1000);
            continue;
        }
        //开始进行处理
        zchxTimeElapsedCounter counter("process video image");
        process(task);
    }
    DEBUG_TRACK_INFO<<"now ZCHXRadarVideoProcessor thread finished...";
}

QColor ZCHXRadarVideoProcessor::getColor(double dValue)
{
    QColor color;
//    if(dValue > 200)
//    {
//        color = QColor(252,241,1);
//    }
//    else if(dValue > 150)
//    {
//        color = QColor(19,118,61);
//    }
//    else if(dValue > 100)
//    {
//        color = QColor(252,241,1);
//    } else
//    {
//        color = QColor(19,118,61);
//    }

    if(dValue > 200)
    {
        color = Qt::red;
    }
    else if(dValue > 150)
    {
        color = Qt::green;
    }
    else if(dValue > 100)
    {
        color = Qt::blue;
    } else
    {
        color = Qt::magenta;
    }

    return color;
}

void ZCHXRadarVideoProcessor::slotSetColor(int a1,int a2,int a3,int b1,int b2,int b3)
{
    m_objColor1 = QColor(a1,a2,a3);
    m_objColor2 = QColor(b1,b2,b3);
}

//将多周期的回波图形合并成一个周期
bool ZCHXRadarVideoProcessor::mergeVideoOfMultiTerms(QMap<int, RADAR_VIDEO_DATA> &result, const ZCHXRadarVideoProcessorData &task)
{
    if(task.size() == 0) return false;
    DEBUG_TRACK_INFO<<"start merge video data of multi terms...";
    result = task[0].m_RadarVideo;
    for(int i=1; i<task.size(); i++)
    {
        if(mIsover) return false;
        QMap<int,RADAR_VIDEO_DATA> tempVideo = task[i].m_RadarVideo;
        for (QMap<int,RADAR_VIDEO_DATA>::iterator it = tempVideo.begin(); it != tempVideo.end(); it++)
        {
            int key = it.key();
            if(mIsover) return false;
            if(result.contains(key))
            {
                QList<int> lineData = it->mLineData;
                //将两个的振幅值进行合并
                RADAR_VIDEO_DATA &old_data = result[key];
                if(old_data.mLineData.size() == lineData.size())
                {
                    for(int i=0; i<lineData.size(); i++)
                    {
                        if(mIsover) return false;
                        if(old_data.mLineData[i] < lineData[i])
                        {
                            old_data.mLineData[i] = lineData[i];
                        }

                    }
                }

            } else
            {
                result[key] = it.value();
            }
        }
    }

    int size = result.size();
    DEBUG_TRACK_INFO<<"end merge video data of multi terms. and result size:"<<size;
    return size > 0;
}

#include <QRunnable>
#include <QThreadPool>
class videoImageDrawRun : public QRunnable
{
public:
    explicit videoImageDrawRun(QPaintDevice *result, const QMap<int, RADAR_VIDEO_DATA> &video, const QColor& color, zchxVideoParserSettings* setting)
    {
        mDev = result;
        mData = video;
        mColor = color;
        mParse = setting;
    }

    ~videoImageDrawRun() {}
    void run()
    {
        if(!mDev) return ;

        QPainter objPainter(mDev);
        objPainter.setRenderHint(QPainter::Antialiasing,true);
        objPainter.translate(QPoint(mDev->width() / 2, mDev->height() / 2));    // 中心点

        //设置颜色,像素点
        int arc_span = qCeil(2.0 * 360 * 16 / 4096) ;
        int point_num = 0;
        QTime t;
        t.start();
        for (QMap<int,RADAR_VIDEO_DATA>::const_iterator it = mData.begin(); it != mData.end(); it++)
        {
            RADAR_VIDEO_DATA data = it.value();

            double dAzimuth = data.m_uAzimuth * (360.0 / 4096)/* + data.m_uHeading*/;

            //这里方位角是相对于正北方向,将他转换到画图的坐标系
            double angle_paint = -270 - dAzimuth;
            int arc_start = qCeil(angle_paint * 16) - arc_span;

            for (int i = 0; i < data.mLineData.size(); i++)
            {
                int position = i;
                int value = data.mLineData[i];
                if(value == 0) continue;
                int min_amplitude = mParse->user_video_parse.amp.min;
                int max_amplitude = mParse->user_video_parse.amp.max;

                if(value<min_amplitude || value > max_amplitude) continue;
                //开始画扫描点对应的圆弧轨迹
                QRect rect(0, 0, 2*position, 2*position);
                rect.moveCenter(QPoint(0, 0));
                QColor objColor = mColor;
                objPainter.setPen(QPen(QColor(objColor), 1));//改像素点
                objPainter.drawArc(rect, arc_start, arc_span);
                point_num++;
            }
        }
        DEBUG_TRACK_INFO<<"total point num = "<<point_num<<" and elapsed:"<<t.elapsed();
    }

private:
    QPaintDevice*                   mDev;
    QMap<int, RADAR_VIDEO_DATA>     mData;
    QColor                          mColor;
    zchxVideoParserSettings*        mParse;
};

bool ZCHXRadarVideoProcessor::drawVideoImage(QPaintDevice *result, const QMap<int, RADAR_VIDEO_DATA> &video, const QColor& color)
{
    if(!result) return false;

    QPainter objPainter(result);
    objPainter.setRenderHint(QPainter::Antialiasing,true);
    objPainter.translate(QPoint(result->width() / 2, result->height() / 2));    // 中心点

    //设置颜色,像素点
    int arc_span = qCeil(2.0 * 360 * 16 / 4096) ;
    for (QMap<int,RADAR_VIDEO_DATA>::const_iterator it = video.begin(); it != video.end(); it++)
    {
        if(mIsover) return false;
        RADAR_VIDEO_DATA data = it.value();

        double dAzimuth = data.m_uAzimuth * (360.0 / 4096)/* + data.m_uHeading*/;

        //这里方位角是相对于正北方向,将他转换到画图的坐标系
        double angle_paint = -270 - dAzimuth;
        int arc_start = qCeil(angle_paint * 16) - arc_span;

        for (int i = 0; i < data.mLineData.size(); i++)
        {
            if(mIsover) return false;
            int position = i;
            int value = data.mLineData[i];
            if(value == 0) continue;
            int min_amplitude = mParse.user_video_parse.amp.min;
            int max_amplitude = mParse.user_video_parse.amp.max;

            if(value<min_amplitude || value > max_amplitude) continue;
            //开始画扫描点对应的圆弧轨迹
            QRect rect(0, 0, 2*position, 2*position);
            rect.moveCenter(QPoint(0, 0));
            QColor objColor = color;
            if(!color.isValid()) objColor = this->getColor(value);
            objPainter.setPen(QPen(QColor(objColor), 1));//改像素点
            objPainter.drawArc(rect, arc_start, arc_span);
        }
    }

    return true;
}

//画回波255位黄色,1-244振幅为蓝色
void ZCHXRadarVideoProcessor::process(const ZCHXRadarVideoProcessorData& task)
{
    zchxTimeElapsedCounter counter(QString(metaObject()->className()) + " : " + QString(__FUNCTION__));
    if(task.size() == 0) return;
    //生成回波的原始图片
    int img_width = (task.first().m_RadarVideo.first().m_uTotalCellNum)*2 - 1;
    int img_height = (task.first().m_RadarVideo.first().m_uTotalCellNum)*2 - 1;
    //    qDebug()<<"image size(w, h):"<<img_width<<img_height;
    QImage objPixmap(img_width, img_height, QImage::Format_ARGB32);
    objPixmap.fill(Qt::transparent);//用透明色填充

    //是否需要几个周期的图片进行叠加
    if(mParse.user_video_parse.merge_video)
    {
        //叠加合并输出
        QMap<int,RADAR_VIDEO_DATA> RadarVideo;
        //合并回波数据
        if(!mergeVideoOfMultiTerms(RadarVideo, task)) return;
        if(!drawVideoImage(&objPixmap, RadarVideo)) return;

    } else
    {
        //开始遍历所有叠加的周期开始画回波图形，每个周期单独绘制
        if(mParse.user_video_parse.video_color_list.size() < task.size())
        {
            DEBUG_TRACK_INFO<<"over lap cnt is larger than color set. anbormal returned";
            return;
        }
        QTime t;
        t.start();
#if 0
        for(int i=0; i<task.size(); i++)
        {
            if(mIsover) return ;
            QColor color;
            color.setNamedColor(mParse.user_video_parse.video_color_list[i].toString());
            if(!drawVideoImage(&objPixmap, task[i].m_RadarVideo, color)) return;
        }
#else

        QThreadPool pool;
        pool.setMaxThreadCount(8);
        QList<QImage*>  imglist;
        for(int i=0; i<task.size(); i++)
        {
            QColor color;
            color.setNamedColor(mParse.user_video_parse.video_color_list[i].toString());
            QImage *img = new QImage(img_width, img_height, QImage::Format_ARGB32);
            img->fill(Qt::transparent);//用透明色填充
            imglist.append(img);
            videoImageDrawRun *run = new videoImageDrawRun(img, task[i].m_RadarVideo, color, &mParse);
            pool.start(run);
        }
        pool.waitForDone();
        DEBUG_TRACK_INFO<<"draw single image:"<<t.elapsed();
        t.restart();
        QPainter painter;
        painter.begin(&objPixmap);
        painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
        foreach (QImage* img, imglist)
        {
            painter.drawImage(0, 0, *img);
            delete img;
        }
        imglist.clear();
#endif
        DEBUG_TRACK_INFO<<"merge img:"<<t.elapsed();
    }

    if(mParse.user_video_parse.use_original_video_img) emit signalSendVideoPixmap(objPixmap);

    if(mTargetExt && !mIsover)
    {
        //对图片进行处理，主要是包括回波过滤，然后提取目标外形点列
        double range_factor = task.first().m_RadarVideo.first().m_dRangeFactor;
        //开始生成任务供目标进行解析
        videoParseData data;
        data.mSrcImg = objPixmap;
        data.mRangeFactor = range_factor;
        data.mTermIndex = task.first().m_VideoTermIndex;
        data.mTermTime = task.first().m_TimeStamp;

        zchxRadarRectDefList list;
        QImage result;
        if(mTargetExt->extractRectFromVideoSrcImg(list, result, data))
        {
            if(!mParse.user_video_parse.use_original_video_img)
            {
                emit signalSendVideoPixmap(result);
            }

            emit signalSendParsedVideoData(list);
        }
    }

    return;
}
