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
#include "zchxradarrectextraction.h"
#include "zchxradartargettrack.h"
#include <QDebug>


ZCHXRadarVideoProcessor::ZCHXRadarVideoProcessor(const zchxVideoParserSettings& setting, QObject *parent)
    : QThread(parent)
    , mTracker(0)
    , mIsover(false)
    , mVideoExtractionWorker(0)
{
    qRegisterMetaType<zchxRadarRectDefList>("const zchxRadarRectDefList&");
    m_objColor1 = QColor(6,144,36);
    m_objColor2 = QColor(103,236,231);
    setStackSize(64000000);
    updateParseSetting(setting);
}


ZCHXRadarVideoProcessor::~ZCHXRadarVideoProcessor()
{
    qDebug()<<"delete rect extrtaction...";
    if(mVideoExtractionWorker)
    {
        delete mVideoExtractionWorker;
    }

    qDebug()<<"delete rect extrtaction end...";
}

void ZCHXRadarVideoProcessor::appendSrcData(const zchxRadarVideoTask &task)
{
    QMutexLocker locker(&mMutex);
    if(mTaskList.size() == 0)
    {
        ZCHXRadarVideoProcessorData data;
        data.append(task);
        mTaskList.append(data);
    } else
    {
        ZCHXRadarVideoProcessorData &data = mTaskList.last();
//        qDebug()<<"last size:"<<data.size()<<mParse.video_overlap_cnt;
        if(data.size() == mParse.video_overlap_cnt)
        {
            //数据已经满足多个周期回波叠加, 构建新的回波数据,新的回波数据以以前的回波数据作为基础
            ZCHXRadarVideoProcessorData newData;
#if 1
            for(int i=1; i<mParse.video_overlap_cnt; i++)
            {
                newData.append(data[i]);
            }
#endif
            newData.append(task);
            mTaskList.append(newData);
        } else
        {
            data.append(task);
        }

        int size = mTaskList.size();
        qDebug()<<"total task list size:"<<size;
//        while (mTaskList.size() > 2) {
//            mTaskList.takeFirst();
//        }
    }
}

bool ZCHXRadarVideoProcessor::getProcessData(ZCHXRadarVideoProcessorData& task)
{
    QMutexLocker locker(&mMutex);
    if(mTaskList.size() == 0) return false;
    ZCHXRadarVideoProcessorData& temp = mTaskList.last();
    if(temp.size() == mParse.video_overlap_cnt)
    {
        task = temp;
#if 1
        //移除以前的任务,保留最近的一个任务,便于下一个回波过来的时候合成新的任务
        while (mTaskList.size() > 1) {
            mTaskList.takeFirst();
        }
        //将任务的第一个回波删除
        temp.takeFirst();
#else
        int size = mTaskList.size();
        qDebug()<<"remove unprocessed video task size:"<<size-1;
        mTaskList.clear();
#endif
        return true;
    }
    return false;
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

        double dAzimuth = data.m_uAzimuth * (360.0 / 4096) + data.m_uHeading;
        //这里方位角是相对于正北方向,将他转换到画图的坐标系
        double angle_paint = -270 - dAzimuth;
        int arc_start = qCeil(angle_paint * 16) - arc_span;

        for (int i = 0; i < data.mLineData.size(); i++)
        {
            if(mIsover) return false;
            int position = i;
            int value = data.mLineData[i];
            if(value == 0) continue;
            int min_amplitude = mParse.amp.min;
            int max_amplitude = mParse.amp.max;

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
    int video_index = task[0].m_IndexT;

    //生成回波的原始图片
    int img_width = (RadarVideo.first().m_uTotalCellNum)*2 - 1;
    int img_height = (RadarVideo.first().m_uTotalCellNum)*2 - 1;
//    qDebug()<<"image size(w, h):"<<img_width<<img_height;
    QImage objPixmap(img_width, img_height, QImage::Format_ARGB32);
    objPixmap.fill(Qt::transparent);//用透明色填充
    if(!drawOriginalVideoImage(&objPixmap, RadarVideo)) return;
    //对图片进行处理，主要是包括回波过滤，然后提取目标外形点列
    double range_factor = RadarVideo.first().m_dRangeFactor;
    QImage result = objPixmap;
    zchxRadarRectDefList list;
    if(mVideoExtractionWorker)
    {
        mVideoExtractionWorker->parseVideoPieceFromImage(result, list, objPixmap, range_factor, video_index, mOutputImg);
    }
    //发送回波矩形集合
    qDebug()<<"parse rect list size:"<<list.size();
    if(list.size() > 0)
    {
        if(!mTracker)
        {
            signalSendRects(list);
        } else
        {
            mTracker->process(list);
        }
    }
    emit signalSendVideoPixmap(result);
    return;
}

void ZCHXRadarVideoProcessor::updateParseSetting(const zchxVideoParserSettings &setting)
{
    mParse = setting;
    //抽出对象初始化
    mOutputImg = mParse.opencv_img;
    if(!mVideoExtractionWorker)
    {
        mVideoExtractionWorker = new zchxRadarRectExtraction(mParse.center_lat, mParse.center_lon);
    } else
    {
        mVideoExtractionWorker->setRadarLL(mParse.center_lat, mParse.center_lon);
    }

    mVideoExtractionWorker->setTargetAreaRange(mParse.area.min, mParse.area.max);
    mVideoExtractionWorker->setTargetLenthRange(mParse.lenth.min, mParse.lenth.max);
    mVideoExtractionWorker->setFilterAreaEnabled(mParse.filter_enable);
    mVideoExtractionWorker->setFilterAreaData(mParse.filter_area);

}
