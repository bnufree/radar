#include "islandlineelement.h"
#include "map_layer/zchxmaplayermgr.h"
#include "zchxmapframe.h"
#include "zchxMapDataUtils.h"

namespace qt {
IslandLineElement::IslandLineElement(const ZCHX::Data::ITF_IslandLine &ele, zchxMapWidget* f)
    : Element(0, 0, f, ZCHX::Data::ELE_ISLAND_LINE, ZCHX::LAYER_ISLAND)
    , m_path(ele.path)
    , m_type((RADARTYPE)ele.type)
{
    if(!ele.path.empty())
    {
        elelat = ele.path.front().first;
        elelon = ele.path.front().second;
    }
    setID(QString::number(ele.id));
    m_data = ele;
    m_id = ele.id;
    m_name = ele.name.toStdString();
    m_content = ele.content.toStdString();
    setIsUpdate(true);
}

std::vector<std::pair<double, double> > IslandLineElement::path() const
{
    return m_data.path;
}

void IslandLineElement::setPath(const std::vector<std::pair<double, double> > &path)
{
    m_data.path = path;
}

//int IslandLineElement::id() const
//{
//    return m_id;
//}

//void IslandLineElement::setId(int id)
//{
//    m_id = id;
//}

std::string IslandLineElement::name() const
{
    return m_name;
}

void IslandLineElement::setName(const std::string &name)
{
    m_name = name;
}

std::string IslandLineElement::content() const
{
    return m_content;
}

void IslandLineElement::setContent(const std::string &content)
{
    m_content = content;
}

void IslandLineElement::setData(const ZCHX::Data::ITF_IslandLine &data)
{
    m_data = data;
    setIsUpdate(true);
}

ZCHX::Data::ITF_IslandLine IslandLineElement::data() const
{
    return m_data;
}

void IslandLineElement::changePathPoint(double lat, double lon)
{
    if(0 <= m_activePathPoint && m_activePathPoint < m_data.path.size())
    {
        m_data.path[m_activePathPoint].first = lat;
        m_data.path[m_activePathPoint].second = lon;
    }
}

void IslandLineElement::moveTo(double lat, double lon)
{
    for(int i= 0; i< m_data.path.size(); ++i)
    {
        m_data.path[i].first  = m_path[i].first  + lat;
        m_data.path[i].second = m_path[i].second + lon;
    }
}

void IslandLineElement::updateOldPath()
{
    m_path = m_data.path;
}

int IslandLineElement::activePathPoint() const
{
    return m_activePathPoint;
}

void IslandLineElement::setActivePathPoint(int activePathPoint)
{
    m_activePathPoint = activePathPoint;
}

void IslandLineElement::delPathPoint(int idx)
{
    if(m_data.path.size() > idx)
    {
        m_data.path.erase(m_data.path.begin()+idx);
        m_path = m_data.path;
        m_activePathPoint = -1;
    }
}

void IslandLineElement::addCtrlPoint(std::pair<double, double> ps)
{
    m_data.path.push_back(ps);
    m_path = m_data.path;
    m_activePathPoint = -1;
}

QString IslandLineElement::color() const
{
    return m_data.warnColor;
}

void IslandLineElement::setColor(QString color)
{
    m_data.warnColor = color;
}

QList<QPoint> IslandLineElement::transfer2Pnts(const QList<QLine> &list) const
{
    QList<QPoint> reslist;
    if(list.size() > 0)
    {
        reslist.append(list.first().p1());
        reslist.append(list.first().p2());
        for(int i=1; i<list.size(); i++)
        {
            QLine line1 = list[i-1];
            QLine line2 = list[i];
            //检查两条线是否有交点
            QPoint cross;
            if(zchxMapDataUtils::is2VectorCross(cross, line1, line2))
            {
                reslist.removeLast();
                reslist.append(cross);
            }
            reslist.append(line2.p1());
            reslist.append(line2.p2());
        }
    }

    return reslist;
}

QPolygon IslandLineElement::shapePoints() const
{
    double width = 10.0;
    std::vector<std::pair<double,double>> tmp_path = path();
    QList<ZCHX::Data::Point2D> center;
    QList<QLine>    left, right;
    for(int i = 0; i < tmp_path.size(); ++i)
    {
        std::pair<double, double> ll = tmp_path[i];
        ZCHX::Data::Point2D  curPos = mView->framework()->LatLon2Pixel(ll.first,ll.second);
        center.append(curPos);
    }
    center.append(center.first());
    for(int i=1; i<center.size(); i++)
    {
        ZCHX::Data::Point2D start = center[i-1];
        ZCHX::Data::Point2D end = center[i];
        double angle = atan2(end.y - start.y, end.x - start.x);
        QLine line(start.x, start.y, end.x, end.y);
        QLine low = line.translated(width * 0.5 * sin(angle), -width *0.5 * cos(angle));
        QLine high = line.translated(-width * 0.5* sin(angle), width *0.5 * cos(angle));
        right.append(low);
        left.append(high);
    }
    //将线转换成
    QList<QPoint> result = transfer2Pnts(right);
    QList<QPoint> result2 = transfer2Pnts(left);
    while (result2.size() > 0) {
        result.append(result2.takeLast());
    }
    return QPolygon(result.toVector());
}

bool IslandLineElement::contains(const QPoint &pt) const
{
    if(!isViewAvailable()) return false;
    if(!mView->getLayerMgr()->isLayerVisible(ZCHX::LAYER_ISLAND)) return false;

    QPolygon    m_shapePnts = shapePoints();
    if(m_shapePnts.last() != m_shapePnts.first()) m_shapePnts.append(m_shapePnts.first());
    bool sts = m_shapePnts.containsPoint(pt, Qt::OddEvenFill);
    //检查当前点离那个距离点最近
    return sts;
}


void IslandLineElement::drawElement(QPainter *painter)
{
    if(!painter || !mView->getLayerMgr()->isLayerVisible(ZCHX::LAYER_ISLAND)) return;
    std::vector<std::pair<double,double>> tmp_path = path();
    QPainterPath polygon;
    for(int i=0; i<tmp_path.size();++i)
    {
        std::pair<double, double> ll = tmp_path[i];
        QPointF pos = mView->framework()->LatLon2Pixel(ll.first, ll.second).toPointF();
        if(0 == i)
        {
            polygon.moveTo(pos);
        }
        else
        {
            polygon.lineTo(pos);
        }
//        polygon.append(pos);
        if(getIsActive())
        {
            PainterPair chk(painter);
            painter->setPen(QPen(Qt::red,1,Qt::SolidLine));
            painter->setBrush(Qt::white);
            painter->drawEllipse(pos,5,5);
        }
    }
    polygon.closeSubpath(); //闭合路径
    PainterPair chk(painter);
    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(QColor(color()),3));
    painter->strokePath(polygon, QPen(QColor(color()),3, Qt::DashDotDotLine));

    std::pair<double, double> ll = tmp_path[0];
    QPointF pos = mView->framework()->LatLon2Pixel(ll.first, ll.second).toPointF();

    painter->drawText(pos, m_data.name);

}
}

