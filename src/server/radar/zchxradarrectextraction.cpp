#include "zchxradarrectextraction.h"
#include <QDebug>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <QPainter>
#include <QDir>
#include <QVector2D>
#include <QCoreApplication>
#include "zchxradarcommon.h"


using namespace std;
using namespace cv;

double polygonArea(const QPolygon& poly)
{
    vector<Point> contour;
    for(int i=0; i<poly.size(); i++)
    {
        QPoint pnt = poly[i];
        contour.push_back(Point2f(pnt.x(), pnt.y()));
    }

    return contourArea(contour);
}

double polygonArea(const QPolygonF& poly)
{
    return polygonArea(poly.toPolygon());
}


cv::Mat QImage2cvMat(QImage image)
{
    cv::Mat mat;
//    qDebug() << image.format();
    switch(image.format())
    {
    case QImage::Format_ARGB32:
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32_Premultiplied:
        mat = cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
        break;
    case QImage::Format_RGB888:
        mat = cv::Mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine());
        cv::cvtColor(mat, mat, CV_BGR2RGB);
        break;
    case QImage::Format_Indexed8:
        mat = cv::Mat(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());
        break;
    }
    return mat;
}

QImage cvMat2QImage(const cv::Mat& mat)
{
    // 8-bits unsigned, NO. OF CHANNELS = 1
    if(mat.type() == CV_8UC1)
    {
        QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
        // Set the color table (used to translate colour indexes to qRgb values)
        image.setColorCount(256);
        for(int i = 0; i < 256; i++)
        {
            image.setColor(i, qRgb(i, i, i));
        }
        // Copy input Mat
        uchar *pSrc = mat.data;
        for(int row = 0; row < mat.rows; row ++)
        {
            uchar *pDest = image.scanLine(row);
            memcpy(pDest, pSrc, mat.cols);
            pSrc += mat.step;
        }
        return image;
    }
    // 8-bits unsigned, NO. OF CHANNELS = 3
    else if(mat.type() == CV_8UC3)
    {
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return image.rgbSwapped();
    }
    else if(mat.type() == CV_8UC4)
    {
        qDebug() << "CV_8UC4";
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
        return image.copy();
    }
    else
    {
        qDebug() << "ERROR: Mat could not be converted to QImage.";
        return QImage();
    }
}

void saveImg(const QString& dir, const QString& name, InputArray mat)
{
    QDir path(QString("%1/temp/%2").arg(QCoreApplication::applicationDirPath()).arg(dir));
    if(!path.exists())
    {
        path.mkpath(path.absolutePath());
    }
    QString filename = QString("%1/%2.png").arg(path.absolutePath()).arg(name);
    imwrite(filename.toStdString(), mat);
}

void parseLatlonJsonArray(const QJsonArray& array, zchxLatlonList& list )
{
    list.clear();
    for(int j = 0; j < array.size(); ++j)
    {
        QJsonArray cellAraay = array.at(j).toArray();
        double dLon = cellAraay.at(0).toDouble();
        double dLat = cellAraay.at(1).toDouble();
        list.append(Latlon(dLat, dLon));
    }
}



zchxRadarRectExtraction::zchxRadarRectExtraction(const zchxVideoParserSettings& settings, QObject *parent)
    : QObject(parent)
    , mSettings(settings)
{
    //对外部设定值进行纠偏处理
    if(mSettings.user_video_parse.area.max == 0) mSettings.user_video_parse.area.max = LONG_MAX;
    if(mSettings.user_video_parse.lenth.max == 0) mSettings.user_video_parse.lenth.max = LONG_MAX;
    initFilterAreaInMector();
}

zchxRadarRectExtraction::~zchxRadarRectExtraction()
{

}


void zchxRadarRectExtraction::initFilterAreaInMector()
{
    if(!mSettings.filter_enable) return;
    for(int i=0; i<mSettings.filter_area_list.size(); i++)
    {
        zchxCommon::zchxFilterArea data = mSettings.filter_area_list[i];
        QPolygonF poly;
        foreach (zchxCommon::zchxLatlon ll, data.area) {
            poly.append(latlonToMercator(ll.lat, ll.lon).toPointF());
        }
        if(poly.size() > 0)
        {

            if(data.type == 0)
            {
                //目标在这些区域要过滤掉，有效目标在这些区域的外面
                mOutFilterAreaMercatorList.append(poly);

            } else
            {
                //目标在这些区域要保留，有效目标在这些区域的里面
                mInFilterAreaMercatorList.append(poly);
            }
        }
    }
}


bool zchxRadarRectExtraction::extractRectFromVideoSrcImg(zchxRadarRectDefList& list,  QImage& result, const videoParseData& data)
{
    zchxTimeElapsedCounter counter(QString(__FUNCTION__));
    int img_width = data.mSrcImg.width();
    int img_height = data.mSrcImg.height();
    //遍历获取到的外形点列,取得目标的中心点
    QList<QPolygon> skip_poly_list;                //过滤区域对应的图形外框

    QTime t;
    t.start();
    //像素坐标系和经纬度转换
    zchxPosConverter posConverter(QPointF(img_width/2.0, img_height/2.0), Latlon(mSettings.center_lat, mSettings.center_lon), data.mRangeFactor);
    //使用opencv的方法将图片抽出目标的外形框
    QString opencv_dir = QString::number(QDateTime::currentMSecsSinceEpoch());
    cv::Mat src = QImage2cvMat(data.mSrcImg);
    if (src.empty()){
        qDebug("colud not load image!!!!");
        return false;
    }
    // 均值降噪.这里的Size的参数影响灰度画处理.数值过大,会使不相干的图片连成一片  干扰目标解析
    Mat blurImg;
    GaussianBlur(src, blurImg, Size(3, 3), 0, 0);

    // 二值化
    Mat binary, gray_src;
    cvtColor(blurImg, gray_src, COLOR_RGB2GRAY);
    threshold(gray_src, binary, 0, 255, THRESH_BINARY /*| THRESH_TRIANGLE*/);

    // 获取最大轮廓
    vector<vector<Point>> contours;
    findContours(binary, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
    if(mSettings.opencv_img)
    {
        Mat connImage = Mat ::zeros(src.size(),CV_8U);
        drawContours(connImage, contours, -1, Scalar(255, 0, 255));
//        saveImg(opencv_dir, "src", src);
//        saveImg(opencv_dir, "blur", blurImg);
//        saveImg(opencv_dir, "gray", gray_src);
//        saveImg(opencv_dir, "binary", binary);
//        saveImg(opencv_dir, "result", connImage);
        saveImg(opencv_dir, "1", src);
        saveImg(opencv_dir, "2", blurImg);
        saveImg(opencv_dir, "3", gray_src);
        saveImg(opencv_dir, "4", binary);
        saveImg(opencv_dir, "5", connImage);
    }
    qDebug()<<"video countours size before filter:"<<contours.size();
    //提取轮廓点
    int index = 1;
    for(int i=0; i<contours.size(); i++)
    {
        vector<Point> pnts_opencv = contours[i];

        //获取目标的外形点列
        QPolygonF mercator_polygonF; //目标对应的像素坐标外形点列，墨卡托坐标外形点列
        QPolygon pixel_polygon;
        zchxLatlonList  latlon_ploygonF;             //目标对应的经纬度坐标外形点列
        for(int k=0; k<pnts_opencv.size(); k++)
        {
            Point2i pnt = pnts_opencv[k];
            pixel_polygon.append(QPoint(pnt.x, pnt.y));
            Latlon ll = posConverter.pixel2Latlon(QPointF(pnt.x, pnt.y));
            latlon_ploygonF.append(ll);
            mercator_polygonF.append(latlonToMercator(ll.lat, ll.lon).toPointF());
        }
        //检查回波图形是否是在禁止区域内,或者禁止区域重叠
        if(!isVideoPolygonAvailable(mercator_polygonF))
        {
            skip_poly_list.append(pixel_polygon);
            continue;
        }
        //计算回波图形的面积,目标的长度和重心位置,检查目标的设定是否设定的阈值范围内
        double area = contourArea(contours[i]);
        //检查面积是否符合要求.将面积转换成米
        area = area * data.mRangeFactor * data.mRangeFactor;
        if(area <= mSettings.user_video_parse.area.min || area >= mSettings.user_video_parse.area.max)
        {
            skip_poly_list.append(pixel_polygon);
            continue;
        }
        //计算雷达目标的长度和宽度,将矩形块二者的最大者作为目标的长度
        RotatedRect rect = minAreaRect(contours[i]);
        double width = rect.size.width;
        double height = rect.size.height;
        double target_length = qMax(width, height) * data.mRangeFactor;
        if(target_length <= mSettings.user_video_parse.lenth.min || target_length >= mSettings.user_video_parse.lenth.max)
        {
            skip_poly_list.append(pixel_polygon);
            continue;
        }
        //计算重心
        Moments M = moments(contours[i]);
        if(area <= 0.0) continue;
        int cx = int(M.m10/M.m00);
        int cy =  int(M.m01/M.m00);
        QPoint grave_pnt(cx, cy);
        grave_pnt.setY(cy);

        //开始生成提取的原始目标
        com::zhichenhaixin::proto::RadarRectDef rectDef;
        //基本信息生成
        rectDef.set_realdata(true);
        rectDef.set_rectnumber(index++);
        rectDef.set_updatetime(data.mTermTime);
        Latlon ll = posConverter.pixel2Latlon(QPointF(cx, cy));
        rectDef.mutable_center()->set_latitude(ll.lat);
        rectDef.mutable_center()->set_longitude(ll.lon);

        //更新目标的外形点列(经纬度点列)
        foreach (Latlon ll, latlon_ploygonF) {
            zchxLatlon *block = rectDef.add_outline();
            block->set_latitude(ll.lat);
            block->set_longitude(ll.lon);
        }
        //重新处理目标的像素点点列，像素点点列的坐标原点为目标的中心位置
        QTransform transform;
        transform.scale(0.4, 0.4);
        transform.translate(-cx, -cy);
        QPolygon new_poly = transform.map(pixel_polygon);
        QSizeF fixSize = new_poly.boundingRect().size();
        rectDef.mutable_fixedimg()->set_width(qRound(fixSize.width()));
        rectDef.mutable_fixedimg()->set_height(qRound(fixSize.height()));
        foreach (QPoint point, new_poly) {
            com::zhichenhaixin::proto::PixelPoint *pnt = rectDef.mutable_fixedimg()->add_points();
            pnt->set_x(point.x());
            pnt->set_y(point.y());
        }
        //设定目标外接矩形
        rectDef.mutable_boundrect()->set_diameter(target_length);
        Rect opencv_bound = rect.boundingRect();
        QRectF bound(opencv_bound.x, opencv_bound.y, opencv_bound.width, opencv_bound.height);
        ll = posConverter.pixel2Latlon(bound.topLeft());
        rectDef.mutable_boundrect()->mutable_topleft()->set_latitude(ll.lat);
        rectDef.mutable_boundrect()->mutable_topleft()->set_longitude(ll.lon);
        ll = posConverter.pixel2Latlon(bound.bottomRight());
        rectDef.mutable_boundrect()->mutable_bottomright()->set_latitude(ll.lat);
        rectDef.mutable_boundrect()->mutable_bottomright()->set_longitude(ll.lon);
        //开始设定目标的长轴坐标
        Point2f rot_center = rect.center;
        double angle = rect.angle * GLOB_PI / 180;
        QPointF startPoint, endPoint;
        startPoint.setX(rot_center.x - rect.size.width * 0.5 * cos(angle));
        startPoint.setY(rot_center.y - rect.size.width * 0.5 * sin(angle));
        endPoint.setX(rot_center.x + rect.size.width * 0.5 * cos(angle));
        endPoint.setY(rot_center.y + rect.size.width * 0.5 * sin(angle));
        ll = posConverter.pixel2Latlon(startPoint);
        rectDef.mutable_seg()->mutable_start()->set_latitude(ll.lat);
        rectDef.mutable_seg()->mutable_start()->set_longitude(ll.lon);
        ll = posConverter.pixel2Latlon(endPoint);
        rectDef.mutable_seg()->mutable_end()->set_latitude(ll.lat);
        rectDef.mutable_seg()->mutable_end()->set_longitude(ll.lon);
        QGeoCoordinate coord1(rectDef.seg().start().latitude(),rectDef.seg().start().longitude());
        QGeoCoordinate coord2(rectDef.seg().end().latitude(),rectDef.seg().end().longitude());
        angle = coord1.azimuthTo(coord2);
        rectDef.mutable_seg()->set_angle(angle);


        rectDef.set_sogms(0.0);
        rectDef.set_cog(0.0);
        rectDef.set_sogknot(0.0);
        rectDef.set_videocycleindex(data.mTermIndex);

        zchxRadarRectDef wrap;
        wrap.mSrcRect.CopyFrom(rectDef);
        wrap.mShapePnts = pixel_polygon;
        wrap.mArea = area;
        wrap.mCenter = grave_pnt;
        wrap.mPosConveter = posConverter;
        list.append(wrap);

    }

    //开始生成新的回波图形
    result = data.mSrcImg.copy();
    QPainter painter;
    painter.begin(&result);

    qDebug()<<"skip poly size:"<<skip_poly_list.size();

    //开始将不符合要求的回波图形删除
    if(skip_poly_list.size() > 0)
    {
        QPainter::CompositionMode old = painter.compositionMode();
        painter.setCompositionMode(QPainter::CompositionMode_Clear);
        painter.setPen(Qt::blue);
        painter.setBrush(Qt::transparent);
        foreach (QPolygonF poly, skip_poly_list) {
            painter.drawPolygon(poly);
        }
        painter.setCompositionMode(old);
    }
    if(0)
    {
        qDebug()<<"filter area size:"<<mOutFilterAreaMercatorList.size()<<mInFilterAreaMercatorList.size();
        //测试禁止区域
        painter.setPen(Qt::blue);
        painter.setBrush(QColor(255, 0, 0, 100));
        foreach (QPolygonF src, mOutFilterAreaMercatorList) {
            QPolygonF dest;
            foreach (QPointF pnt, src) {
                Latlon ll = mercatorToLatlon(Mercator(pnt));
                dest.append(posConverter.Latlon2Pixel(ll));
            }
            painter.drawPolygon(dest);
        }

        painter.setBrush(QColor(0, 255, 0, 100));
        foreach (QPolygonF src, mInFilterAreaMercatorList) {
            QPolygonF dest;
            foreach (QPointF pnt, src) {
                Latlon ll = mercatorToLatlon(Mercator(pnt));
                dest.append(posConverter.Latlon2Pixel(ll));
            }
            painter.drawPolygon(dest);
        }
    }
    painter.end();
    if(mSettings.opencv_img)
    {
        saveImg(opencv_dir, "final", QImage2cvMat(result));
    }

    //开始画连续5个周期的回波图形
    if(0)
    {
        mDrawVideoList.append(list);
        if(mDrawVideoList.size() > 5) mDrawVideoList.takeFirst();
        QImage objPixmap(img_width, img_height, QImage::Format_ARGB32);
        objPixmap.fill(Qt::transparent);//用透明色填充
        QPainter painter;
        painter.begin(&objPixmap);
        QList<QColor> color_list;
        color_list<<QColor(Qt::red)<<QColor(Qt::green)<<QColor(Qt::yellow)<<QColor(Qt::magenta)<<QColor(Qt::blue);
        for(int i=0; i<mDrawVideoList.size(); i++)
        {
            QColor color = color_list[i];
            color.setAlpha(100);
            painter.setBrush(color);
            foreach (zchxRadarRectDef rect, mDrawVideoList[i]) {
                double lat = rect.mSrcRect.center().latitude();
                double lon = rect.mSrcRect.center().longitude();
                QPointF pixel = posConverter.Latlon2Pixel(Latlon(lat, lon));
                painter.drawEllipse(pixel, 3, 3);
            }
        }
        painter.end();

        QDir path(QString("%1/temp").arg(QCoreApplication::applicationDirPath()));
        if(!path.exists())
        {
            path.mkpath(path.absolutePath());
        }
        QString filename = QString("%1/%2.png").arg(path.absolutePath()).arg(QDateTime::currentMSecsSinceEpoch());
        imwrite(filename.toStdString(), QImage2cvMat(objPixmap));
    }

    return list.size() != 0;
}

//区域限制功能
bool zchxRadarRectExtraction::isVideoPolygonAvailable(const QPolygonF& mercator_polygon)
{
    if(!mSettings.filter_enable) return true;
    //检查区域是否在指定的过滤区域内

    //先检查水域
    bool in_sea = false;
    if(mInFilterAreaMercatorList.size() == 0)
    {
        //未指定水域的情况，默认都在水域内
        in_sea = true;
    } else
    {
        foreach (QPolygonF src, mInFilterAreaMercatorList) {
            if(src.intersected(mercator_polygon).size() > 0 && mercator_polygon.subtracted(src).size() == 0)
            {
                in_sea = true;
                break;
            }
        }
    }
    if(!in_sea) return false;//没有在海里,直接返回不符合要求
    //目标符合在水里，检查是否在指定的排除区域内
    foreach (QPolygonF src, mOutFilterAreaMercatorList) {
        if(src.intersected(mercator_polygon).size() > 0)
        {
            //和排除区域相交了，不符合要求
            return false;
        }
    }
    return true;
}








