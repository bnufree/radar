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
    qDebug()<<"after append task size:"<<mTaskList.size()<<" overlap cnt:"<<mParse.user_video_parse.video_overlap_cnt;
}

bool ZCHXRadarVideoProcessor::getProcessData(ZCHXRadarVideoProcessorData& task)
{
    QMutexLocker locker(&mMutex);
    int size = mTaskList.size();
    if(size < mParse.user_video_parse.video_overlap_cnt) return false;
    int start_index = size - mParse.user_video_parse.video_overlap_cnt;
    task = mTaskList.mid(start_index, mParse.user_video_parse.video_overlap_cnt);
    mTaskList = mTaskList.mid(start_index + 1);
    qDebug()<<"task left size:"<<mTaskList.size()<<" overlap cnt:"<<mParse.user_video_parse.video_overlap_cnt;
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
    qDebug()<<"now thread finished...";
}

QColor ZCHXRadarVideoProcessor::getColor(double dValue)
{
    QColor color;
    if(dValue > 200)
    {
        color = QColor(252,241,1);
    }
    else if(dValue > 150)
    {
        color = QColor(19,118,61);
    }
    else if(dValue > 100)
    {
        color = QColor(252,241,1);
    } else
    {
        color = QColor(19,118,61);
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
    qDebug()<<"start merge video data of multi terms...";
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
    qDebug()<<"end merge video data of multi terms. and result size:"<<size;
    return size > 0;
}

bool ZCHXRadarVideoProcessor::drawOriginalVideoImage(QPaintDevice *result, const QMap<int, RADAR_VIDEO_DATA> &video)
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
            QColor objColor = this->getColor(value);
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
    QMap<int,RADAR_VIDEO_DATA> RadarVideo;
    //合并回波数据
    if(!mergeVideoOfMultiTerms(RadarVideo, task)) return;

    //生成回波的原始图片
    int img_width = (RadarVideo.first().m_uTotalCellNum)*2 - 1;
    int img_height = (RadarVideo.first().m_uTotalCellNum)*2 - 1;
//    qDebug()<<"image size(w, h):"<<img_width<<img_height;
    QImage objPixmap(img_width, img_height, QImage::Format_ARGB32);
    objPixmap.fill(Qt::transparent);//用透明色填充
    if(!drawOriginalVideoImage(&objPixmap, RadarVideo)) return;
    if(mParse.user_video_parse.use_original_video_img) emit signalSendVideoPixmap(objPixmap);

    if(mTargetExt)
    {
        //对图片进行处理，主要是包括回波过滤，然后提取目标外形点列
        double range_factor = RadarVideo.first().m_dRangeFactor;
        //开始生成任务供目标进行解析
        videoParseData data;
        data.mSrcImg = objPixmap;
        data.mRangeFactor = range_factor;
        data.mTermIndex = task.first().m_IndexT;
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
