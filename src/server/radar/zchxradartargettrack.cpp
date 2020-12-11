#include "zchxradartargettrack.h"
#include <QDebug>
#include <QGeoCoordinate>

const bool track_debug = false;

const double point_near_line = 200;
const double ship_max_speed = 5.0;            //5m/s ~~ 10节  10m/s 就是20节


#define     DEBUG_TRACK_INFO                0


//double getDeltaTime(float now, float old)
//{
//    double delta = now - old;
//    if(delta < 0)
//    {
//        delta += 3600 * 24;
//    }
//    return delta;
//}
extern bool restart_simulate;
extern int  target_silent_confirm_counter;
extern bool output_silent_node;

QPolygonF  predictionAreaLL2PolygonF(const com::zhichenhaixin::proto::PredictionArea& area)
{
    QPolygonF poly;
    for(int i=0; i<area.area_size(); i++)
    {
        com::zhichenhaixin::proto::Latlon pnt = area.area(i);
        poly.append(latlonToMercator(pnt.latitude(), pnt.longitude()).toPointF());
    }

    return poly;
}
zchxRadarTargetTrack::zchxRadarTargetTrack(const zchxVideoParserSettings& setting, QObject *parent)
#ifdef TRACK_THREAD
    : QThread(parent)
#else
    : QObject(parent)
#endif

    , mSettings(setting)
    , mMaxEstCount(5)
    , mTargetPredictionInterval(2)
{
    mMinNum = (setting.radar_id * 10 + setting.channel_id) * 10000;
    mMaxNum = mMinNum + 9998;
    mRectNum = mMinNum;
    target_silent_confirm_counter = setting.confirm_target_cnt;
    qRegisterMetaType<zchxRadarSurfaceTrack>("const zchxRadarSurfaceTrack&");
}

zchxRadarTargetTrack::~zchxRadarTargetTrack()
{
    mTargetNodeMap.clear();
}

void zchxRadarTargetTrack::appendTask(const zchxRadarRectDefList &task)
{
    QMutexLocker locker(&mMutex);
    mTaskList.append(task);
}

bool zchxRadarTargetTrack::getTask(zchxRadarTrackTask &task)
{
    QMutexLocker locker(&mMutex);
    if(mTaskList.size() == 0) return false;
    task = mTaskList.takeLast();
    if(mTaskList.size() > 0)
    {
        qDebug()<<"delete unprocessed task size:"<<mTaskList.size();
        mTaskList.clear();
    }

}

#ifdef TRACK_THREAD
void zchxRadarTargetTrack::run()
{
    while (true) {
        //获取当前的任务
        zchxRadarTrackTask task;
        if(!getTask(task))
        {
            msleep(1000);
            continue;
        }
        //开始进行处理
        process(task);
    }
}
#endif

void zchxRadarTargetTrack::mergeRectTargetInDistance(zchxRadarTrackTask &temp_list, int target_merge_distance)
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    //目标预处理.将距离太近的目标进行合并,合并的中心点取二者的重点
    for(int i=0; i<temp_list.size();)
    {
        zchxRadarRectDef &cur = temp_list[i];
        //获取和当前目标距离较近的目标,目标可能多个,这里暂且设定为list,
        zchxRadarRectDefList merge_list;
        for(int k = i+1; k<temp_list.size();)
        {
            zchxRadarRectDef next = temp_list[k];
            //计算两者的距离
            double distance = getDisDeg(cur.center().latitude(), cur.center().longitude(),
                                        next.center().latitude(), next.center().longitude());
            if(distance < target_merge_distance)
            {
                //目标存在,从原来的队列里边删除
                merge_list.append(next);
                temp_list.removeAt(k);
                continue;
            }
            k++;
        }
        if(merge_list.size() == 0)
        {
            i++;
            continue;
        }
        //合并目标
        merge_list.append(cur);
        Latlon ll = getMergeTargetLL(merge_list);
        changeTargetLL(ll, cur);
    }
}

Latlon zchxRadarTargetTrack::getMergeTargetLL(const zchxRadarRectDefList &list)
{
    double sum_x = 0, sum_y = 0;
    foreach (zchxRadarRectDef temp, list) {
        sum_x += temp.center().longitude();
        sum_y += temp.center().latitude();
    }
    if(list.size() > 0)
    {
        sum_x = sum_x / list.size();
        sum_y = sum_y / list.size();
    }

    return Latlon(sum_y, sum_x);
}

void zchxRadarTargetTrack::changeTargetLL(const Latlon &ll, zchxRadarRectDef &cur)
{
    double dlon = ll.lon - cur.center().longitude();
    double dlat = ll.lat - cur.center().latitude();
    cur.mutable_center()->set_longitude(ll.lon);
    cur.mutable_center()->set_latitude(ll.lat);
    //将目标的所有经纬度坐标进行同步的变换
    cur.mutable_boundrect()->mutable_topleft()->set_latitude(cur.boundrect().topleft().latitude() + dlat);
    cur.mutable_boundrect()->mutable_topleft()->set_longitude(cur.boundrect().topleft().longitude() + dlon);
    cur.mutable_boundrect()->mutable_bottomright()->set_latitude(cur.boundrect().bottomright().latitude() + dlat);
    cur.mutable_boundrect()->mutable_bottomright()->set_longitude(cur.boundrect().bottomright().longitude() + dlon);
    for(int i=0; i<cur.outline_size(); i++)
    {
        zchxLatlon *ll = cur.mutable_outline(i);
        ll->set_latitude(ll->latitude() + dlat);
        ll->set_longitude(ll->longitude() + dlon);
    }
    if(cur.has_seg())
    {
        cur.mutable_seg()->mutable_start()->set_latitude(cur.seg().start().latitude() + dlat);
        cur.mutable_seg()->mutable_start()->set_longitude(cur.seg().start().longitude() + dlon);
        cur.mutable_seg()->mutable_end()->set_latitude(cur.seg().end().latitude() + dlat);
        cur.mutable_seg()->mutable_end()->set_longitude(cur.seg().end().longitude() + dlon);
    }
}

bool zchxRadarTargetTrack::isDirectionChange(double src, double target)
{
    //计算原角度对应的相反角度的范围,反映到0-360的范围
    int min = int(src - mSettings.direction_invert_hold);
    int max = int(src + mSettings.direction_invert_hold);
    if(min < 0)
    {
        //原方向指向右上方向,则有效值的范围是min_+360 ~ 360, 0~max
        if((target >= 0  && target < max) || (target > min+360)) return false;
        return true;
    } else if(max > 360)
    {
        //原方向指向左上方向,则有效值的范围是min ~ 360, 0~max-360
        if((target >= 0  && target < max-360) || (target > min)) return false;
        return true;
    } else
    {
        //最大或者最小都在0-360的范围内
        if(target > min && target < max) return false;
        return true;
    }
    return false;
}

bool zchxRadarTargetTrack::isRectAreaContainsPoint(const zchxRadarRectDef &rect, double lat, double lon)
{
    bool sts = false;
    QPolygonF poly;
    for(int i=0; i<rect.outline_size();i++)
    {
        zchxLatlon block = rect.outline(i);
        //将经纬度转换成墨卡托点列
        Mercator m = latlonToMercator(block.latitude(), block.longitude());
        poly.append(m.toPointF());
    }
    if(poly.size() > 0)
    {
        sts = poly.containsPoint(latlonToMercator(lat, lon).toPointF(), Qt::OddEvenFill);
    }
    return sts;
}

void zchxRadarTargetTrack::processWithPossibleRoute(const zchxRadarTrackTask &task)
{
    if(task.size() == 0) return;
    int cur_cycle_index = task.first().videocycleindex();
//    zchxTimeElapsedCounter counter(__FUNCTION__);
    zchxRadarRectDefList temp_list(task);             //保存的未经处理的所有矩形单元
    quint32 now_time = task.first().updatetime();
   if(track_debug) qDebug()<<"now process list time:"<<QDateTime::fromTime_t(task.first().updatetime()).toString("yyyy-MM-dd hh:mm:ss")<<task.size();


    QTime t;
    int elapsed = 0;
    t.start();
    //默认扫描周期是3s，最大速度是10m/s
    double max_speed = mSettings.max_target_speed * 1.852 / 3.6;
    QList<AreaNodeTable> areaTableList = calculateTargetTrackMode(max_speed, task.first().updatetime(), mSettings.scan_time);
    QList<int>  used_index_list;

    elapsed = t.elapsed();
    if(track_debug) qDebug()<<metaObject()->className()<<__FUNCTION__<<" calculate prediction area elaped:"<<elapsed;
    t.start();

    if(areaTableList.size() > 0)
    {
        //先检查每一个区域内落入了哪些目标
        for(int i=0; i<areaTableList.size(); i++)
        {
            AreaNodeTable &table = areaTableList[i];
            for(int k=0; k<temp_list.size(); k++)
            {
                zchxRadarRectDef def = temp_list[k];
                Mercator pos = latlonToMercator(def.center().latitude(), def.center().longitude());
                if(table.mArea.containsPoint(pos.toPointF(), Qt::OddEvenFill))
                {
                    table.mRectList.append(def);
                }
            }
            //打印每一个预推区域对应的回波中心矩形
            if(DEBUG_TRACK_INFO)
            {
                QStringList node_number_list;
                foreach (TargetNode* node, table.mNodeList) {
                    node_number_list.append(QString::number(node->mSerialNum));
                }
                QStringList rectNumList;
                foreach (zchxRadarRectDef def, table.mRectList) {
                    rectNumList.append(QString::number(def.rectnumber()));
                }
                if(track_debug) qDebug()<<"prediction["<<i+1<<"] has node list:"<<node_number_list.join(" ")<<" rect num:"<<rectNumList.join(" ");
            }
        }
        //开始统计整理，每一个节点现在有多少个待选节点
        QMap<TargetNode*, zchxRadarRectDefList> counter_map;
        QMap<int, QList<int>>                   used_node_rectMap;
        foreach (AreaNodeTable table, areaTableList) {
            foreach (TargetNode* node, table.mNodeList) {
#if 1
                counter_map[node].append(table.mRectList);
#else
                //这里需要添加矩形重复的过滤，避免多次重复
                foreach (zchxRadarRectDef def, table.mRectList) {
                    if(used_node_rectMap.contains(node->mSerialNum) &&
                            used_node_rectMap[node->mSerialNum].contains(def.rectnumber()))
                    {
                        continue;
                    }
                    counter_map[node].append(def);
                    used_node_rectMap[node->mSerialNum].append(def.rectnumber());
                }
#endif
            }
        }
        if(DEBUG_TRACK_INFO)
        {
            QMap<TargetNode*, zchxRadarRectDefList>::iterator it = counter_map.begin();
            for(; it != counter_map.end(); it++)
            {
                TargetNode* node = it.key();
                if(!node) continue;
                QStringList rectNumList;
                foreach (zchxRadarRectDef def, it.value()) {

                        rectNumList.append(QString::number(def.rectnumber()));
                }

                if(track_debug) qDebug()<<"node: "<<node->mSerialNum<<" has releated rect:"<<rectNumList.join(",");
            }
        }
        //开始将新的目标更新到旧目标对应的路径
        struct DetermineNode{
            bool selectedRealNode;
            zchxRadarRectDefList targetList;
        };

        QMap<TargetNode*, zchxRadarRectDefList>::iterator it = counter_map.begin();
        for(; it != counter_map.end(); it++)
        {
            TargetNode* node = it.key();
            if(!node) continue;
            zchxRadarRectDefList targetList = it.value();
            TargetNode *topNode = node->topNode();
            int path_size = 0;
            if(topNode) path_size = topNode->mChildren.size();

            DetermineNode determination;

            if(targetList.size() > 0)
            {
                determination.selectedRealNode = true;
                if(DEBUG_TRACK_INFO)
                {
                    if(track_debug) qDebug()<<"node: "<<node->mSerialNum<<" is about to find the corresponding rect. rect size:"<<targetList.size();
                }

                if(node->mParent)
                {
                    if(track_debug) qDebug()<<"node: "<<node->mSerialNum<<" is a child. find the closest rect"<<" path size:"<<path_size;
                    //如果节点是目标路径上的子节点，那么候选目标就只有一个，选择距离预推运动点最近的目标
                    int    index = -1;
                    double delta_time = now_time - node->mDefRect->updatetime();
                    QGeoCoordinate source(node->mDefRect->center().latitude(), node->mDefRect->center().longitude());
                    //计算目标的预推位置
                    double delta_dis = node->mDefRect->sogms() * delta_time;
                    QGeoCoordinate dest = source.atDistanceAndAzimuth(delta_dis, node->mDefRect->cog());
                    int    min_dis = INT32_MAX;
                    QList<int>  index_list;
                    for(int i=0; i<targetList.size();i++)
                    {
                        zchxRadarRectDef def = targetList[i];
                        if(index_list.contains(def.rectnumber())) continue;
                        index_list.append(def.rectnumber());
                        double dis = dest.distanceTo(QGeoCoordinate(def.center().latitude(), def.center().longitude()));
                        if(dis < min_dis)
                        {
                            min_dis = dis;
                            index = i;
                        }
                    }
                    zchxRadarRectDef def =  targetList.takeAt(index);
                    targetList.clear();
                    targetList.append(def);
                } else
                {
                    //如果节点是第一个节点，那么目标的候选节点可以多个，暂且将所有点都添加到目标的候选中。
                    //这里就不进行任何其他操作
                    if(track_debug) qDebug()<<"node: "<<node->mSerialNum<<" is a top node. all coresspond rect appended"<<" path size:"<<path_size<<" to be added targe size:"<<targetList.size();
                }
                determination.targetList.append(targetList);

                if(DEBUG_TRACK_INFO)
                {
                    QStringList rectNumList;
                    foreach (zchxRadarRectDef def, targetList) {

                            rectNumList.append(QString::number(def.rectnumber()));
                    }

                    if(track_debug) qDebug()<<"node: "<<node->mSerialNum<<" has select rect:"<<rectNumList;
                }
            } else
            {
                determination.selectedRealNode = false;
                //预推区域内没有找到目标图形，现在看看是否在一个回波图形上
                if(DEBUG_TRACK_INFO) qDebug()<<"node "<<node->mSerialNum<<" has no selected rect. now check whether it is in an exist rect.";
                //检查当前目标是否落在了某一个回波图形上
                for(int i=0; i<temp_list.size(); i++)
                {
                    zchxRadarRectDef rect = temp_list[i];
                    if(isRectAreaContainsPoint(rect, node->mDefRect->center().latitude(), node->mDefRect->center().longitude()))
                    {
                        if(DEBUG_TRACK_INFO)    qDebug()<<"find node in rect. node_number:"<<node->mSerialNum<<" origin rect number:"<<rect.rectnumber();
                        targetList.append(rect);
                        break;
                    }
                }
                if(targetList.size() > 0)
                {
                    determination.targetList.append(targetList);
                }
            }
            if(determination.targetList.size() == 0) continue;


            //根据候选目标来源的不同，进行分别处理
            if(determination.selectedRealNode)
            {
                if(DEBUG_TRACK_INFO) qDebug()<<"update current node with rect in its prediction area";
                //目标来源于真实的路径预推，直接更新到对应的路径
                foreach (zchxRadarRectDef target, determination.targetList) {
                    //获取选择的目标，更新速度等
                    if(!used_index_list.contains(target.rectnumber()))
                    {
                        if(DEBUG_TRACK_INFO) qDebug()<<"add origin rect into used list , not make new node from it."<<target.rectnumber();
                        used_index_list.append(target.rectnumber());
                    }
                    if(node->containsRect(target)) continue;

                    target.set_realdata(true);
                    QGeoCoordinate source(node->mDefRect->center().latitude(), node->mDefRect->center().longitude());
                    QGeoCoordinate dest(target.center().latitude(), target.center().longitude());
                    double delta_time = now_time - node->mDefRect->updatetime();
                    double cog = source.azimuthTo(dest);
                    double distance = source.distanceTo(dest);
                    double sog = 0.0;
                    if(delta_time > 0) sog = distance / delta_time;
                    //开始平均速度和角度
                    double refer_sog = node->getReferenceSog();
                    if(refer_sog > 0.1)
                    {
                        sog = (sog + refer_sog) / 2.0;
                    }
                    target.set_cog(cog);
                    target.set_sogms(sog);
                    bool ok = true;
                    if(node->mDefRect->sogms() > 1.0 && (sog > node->mDefRect->sogms() * 2.0)) ok = false;
                    if(1)
                    {
                        node->mChildren.append(QSharedPointer<TargetNode>(new TargetNode(target, node)));
                        if(node->isTopNode()){
                            if(DEBUG_TRACK_INFO) qDebug()<<"top node. node number: "<<node->mSerialNum<<" chiildren size:"<<node->mChildren.size()<<" directly"<<target.rectnumber();
                        }
                        if(DEBUG_TRACK_INFO) qDebug()<<"add child node into path. node number: "<<node->mSerialNum<<" rect number:"<<target.rectnumber();
                    }
                 }

            } else
            {
                if(DEBUG_TRACK_INFO) qDebug()<<"update current node with rect for node is rect area...this wanna not see. ";
                //目标节点本身没有更新，只是他现在处在一个回波图形上，如果他不更新，则有可能同一个回波图形在不同的位置出现了多个目标，
                //为了避免这种情况,我们还是假定目标移动到了回波图形的中心。

                zchxRadarRectDef target = targetList.first();
                if(!used_index_list.contains(target.rectnumber()))
                {
                    if(DEBUG_TRACK_INFO) qDebug()<<"add origin rect into used list , not make new node from it."<<target.rectnumber();
                    used_index_list.append(target.rectnumber());
                }
                target.set_realdata(true);
                //根据不同的节点位置来进行区分，如果目标所在的路径只有一个节点，则直接将目标移动过去。
                //如果目标的路径上存在多个节点，如果在目标的运动方向上，则添加到目标的子节点，
                //如果在运动的反向上，则删除前面的节点，使得更新的节点和前面的节点看起来方向一致
                if(!node->mParent)
                {
                    //单独的目标，直接目标位置移动
                    //节点本身就是根节点，不用计算速度角度
                    target.set_sogms(0.0);
                    target.set_cog(0.0);
                    if(DEBUG_TRACK_INFO) qDebug()<<"move single node "<<node->mSerialNum<<" to including rect center. rect number:"<<target.rectnumber();
                    node->mDefRect->CopyFrom(target);
                    node->mUpdateTime = target.updatetime();
                    node->mVideoIndexList.append(target.videocycleindex());
                } else
                {
                    if(DEBUG_TRACK_INFO) qDebug()<<"fina a target in the path whose cog is same as the updated one. we will move up up to the parent";
                    //计算新位置对应的目标方向
                    QGeoCoordinate source(node->mDefRect->center().latitude(), node->mDefRect->center().longitude());
                    QGeoCoordinate dest(target.center().latitude(),target.center().longitude());
                    double cog = source.azimuthTo(dest);
                    //目标方向是否与原来的方向相同
                    TargetNode *startNode = 0;
                    if(!isDirectionChange(node->mDefRect->cog(), cog))
                    {
                        //目标相同
                        startNode = node;
                        if(DEBUG_TRACK_INFO)qDebug()<<"current node cog is ok, go on";
                    } else
                    {
                        if(DEBUG_TRACK_INFO)qDebug()<<"current node cog is wrong, we need to move up up";
                        //方向反了，从上寻找可能的开始位置
                        //这里需要排除根节点，根节点没有方向。如果遍历到了根节点，则直接将目标更新到根节点，删除原来的根节点下的子节点数据
                        TargetNode* child = node;
                        TargetNode* parent = node->mParent;
                        while (parent) {
                            if(parent->isTopNode())
                            {
                                if(DEBUG_TRACK_INFO)qDebug()<<"now we choose the top child for others not found in the path.";
                                startNode = parent;
                                //删除当前节点所在的分子
                                break;
                            }
                            source.setLatitude(parent->mDefRect->center().latitude());
                            source.setLongitude(parent->mDefRect->center().longitude());
                            cog = source.azimuthTo(dest);
                            if(!isDirectionChange(parent->mDefRect->cog(), cog))
                            {
                                startNode = parent;
                                break;
                            }
                            child = parent;
                            parent = parent->mParent;
                        }
                        if(startNode)
                        {
                            if(DEBUG_TRACK_INFO) qDebug()<<"remove child for path direction adjustment";
                            //将原来的子节点删除
                            startNode->removeChild(child);
                        }
                    }
                    if(startNode)
                    {
                        //检查目标的子类节点是否已经包含了当前的矩形目标，如果包含了就不再添加了。
                        if(!startNode->containsRect(target))
                        {

                            source.setLatitude(startNode->mDefRect->center().latitude());
                            source.setLongitude(startNode->mDefRect->center().longitude());
                            double distance = source.distanceTo(dest);
                            int delta_time = target.updatetime() - startNode->mDefRect->updatetime();
                            double sog = 0.0;
                            if(delta_time > 0) sog = distance / delta_time;
                            //开始平均速度和角度
                            double refer_sog = startNode->getReferenceSog();
                            if(refer_sog > 0.1)
                            {
                                sog = (sog + refer_sog) / 2.0;
                            }
                            target.set_cog(cog);
                            target.set_sogms(sog);
                            bool ok = true;
                            if(startNode->mDefRect->sogms() > 1.0 && (sog > startNode->mDefRect->sogms() * 2.0)) ok = false;
                            if(1)
                            {
                                startNode->mChildren.append(QSharedPointer<TargetNode>(new TargetNode(target, startNode)));
                                if(startNode->isTopNode() && DEBUG_TRACK_INFO) qDebug()<<"top node. node number: "<<startNode->mSerialNum<<" chiildren size:"<<startNode->mChildren.size()<<" indirectly"<<target.rectnumber();
                                if(DEBUG_TRACK_INFO) qDebug()<<"add child node into path. node number: "<<startNode->mSerialNum<<" including rect number:"<<target.rectnumber();
                            }
                        }

                    }
                }
            }
        }
    }
    elapsed = t.elapsed();
    if(track_debug) qDebug()<<metaObject()->className()<<__FUNCTION__<<" update exist obj elaped:"<<elapsed;
    t.start();

    //将未更新的矩形作为新目标添加
    for(int i=0; i<temp_list.size(); i++)
    {
        zchxRadarRectDef rect = temp_list[i];
        if(used_index_list.contains(rect.rectnumber())) continue;        
        TargetNode *node = new TargetNode(rect);        
        appendNode(node, 0);
        if(DEBUG_TRACK_INFO)qDebug()<<"make new node from origin rect ."<<"node number:"<<node->mSerialNum<<"origin rect number:"<<rect.rectnumber();
    }

    //开始检查已经存在的目标的更新状态（静止，运动，还是不能确定），如果路径节点数大于N，则目标确认输出。目标位置变化不大，就是静止，否则就是运动
    QList<int> exist_numbers = mTargetNodeMap.keys();
    foreach (int key, exist_numbers)
    {
        QSharedPointer<TargetNode> node = mTargetNodeMap[key];
        if(!node || node->mChildren.size() == 0) continue;
        //检查目标信息,查看目标的可能路径是否已经满足确定条件，
        if(node->mStatus  != Node_UnDef) continue;  //目标的路径信息已经确定，不处理
        TargetNode* move_child_node = 0;
        for(int i=0; i<node->mChildren.size(); i++)
        {
            //从这个子节点开始遍历，开始构造路径
            QList<TargetNode*> list;
            list.append(node.data());
            TargetNode* current = node->mChildren[i].data();
            if(!current) continue;
            while (current) {
                list.append(current);
                if(current->mChildren.size() == 0) break;
                current = current->mChildren.first().data();
            }
            QList<TargetNode*> new_path_list;
            NodeStatus sts = checkRoutePathSts(new_path_list, list);
            if(sts == Node_Moving)
            {
                if(DEBUG_TRACK_INFO) qDebug()<<"find obj moveing:"<<node->mSerialNum;
                move_child_node = node->mChildren[i].data();
                node->mStatus = sts;
            } else if(sts == Node_GAP_ABNORMAL)
            {
                //目标本身已经符合运动条件，但是突然出现目标之间的距离变化较大，超出了原来的2倍或者2分之一，这里从开始变化的点开始独立出来作成新的待确定目标
                QSharedPointer<TargetNode> topNode = node->mChildren.at(i);
                TargetNode* refer_node = new_path_list.first();
                bool found = false;
                while (topNode)
                {
                    if(topNode.data() == refer_node)
                    {
                        found = true;
                        if(topNode.data()->mParent)
                        {
                            topNode.data()->mParent->removeChild(refer_node);
                        }
                        break;
                    }
                    if(topNode->mChildren.size() == 0) break;
                    topNode = topNode->mChildren.first();
                }
                if(found)
                {
                    topNode.data()->mStatus = Node_UnDef;
                    topNode.data()->mParent = 0;
                    topNode.data()->mDefRect->set_sogms(0.0);
                    topNode.data()->mDefRect->set_cog(0.0);
                    int node_num = getCurrentNodeNum();
                    topNode->mSerialNum = node_num;
                    topNode->mPredictionNode = 0;
                    topNode->clearPrediction();
                    mTargetNodeMap.insert(node_num, topNode);
                }

            } else if(new_path_list.size() > 0)
            {
                //路径的方向混乱，重新按照最后的方向构造
                QSharedPointer<TargetNode> topNode = node->mChildren.at(i);
                TargetNode* refer_node = new_path_list.first();
                bool found = false;
                while (topNode)
                {
                    if(topNode.data() == refer_node)
                    {
                        found = true;
                        break;
                    }
                    if(topNode->mChildren.size() == 0) break;
                    topNode = topNode->mChildren.first();
                }
                if(found)
                {
                    topNode.data()->mStatus = Node_UnDef;
                    topNode.data()->mParent = 0;
                    int node_num = getCurrentNodeNum();
                    topNode->mSerialNum = node_num;
                    topNode->mPredictionNode = 0;
                    topNode->clearPrediction();
                    mTargetNodeMap.insert(node_num, topNode);
                    //删除这一条路径
                    node->mChildren.removeAt(i);
                    i--;
                }
            }
            if(move_child_node) break;
        }
        if(move_child_node)
        {
            //开始将确认的没有确认的路径进行重新分离编号
            splitAllRoutesIntoTargets(node.data(), move_child_node);
            node->updateRouteNodePathStatus(node->mStatus);
        }
    }

    elapsed = t.elapsed();
    if(track_debug) qDebug()<<metaObject()->className()<<__FUNCTION__<<" update obj status elaped:"<<elapsed;
    t.start();

    //这里对目标进行总的遍历处理
    //1)将目标的时间更新到目标的最后节点的时间
    //2)获取目标对应的回波周期序列，确定静止目标是不是虚警。
    //3)统计每一个回波图形对应了多少个目标，严格说来，每一个回波图形只能有一个目标。
    //  如果存在多个目标，则有可能是多个目标同时因为相交相遇追越等原因更新到了一起.
    //  检查目标的运动方向和速度，如果两个都相同，则认为是同一个目标

    QMap<int, QList<TargetNode*>> counterNode;  //key为当前矩形的编号
    QList<int>      deleteNodeList;
    foreach (QSharedPointer<TargetNode> node, mTargetNodeMap)
    {
        if(!node) continue;
        node->setAllNodeSeriaNum(node->mSerialNum);
        uint child_time = node->getLatestChildUpdateTime();
        //更新时间
        if(node->mUpdateTime < child_time) node->mUpdateTime = child_time;
        //虚警判断
        node->mFalseAlarm = node->isFalseAlarm(cur_cycle_index);
        //寻找重复目标（速度和方向都相同）
        QList<TargetNode*> children = node->getAllBranchLastChild();
        if(children.size() > 1)
        {
            //目标现在还没有输出，不处理
            continue;
        }
        TargetNode* checkNode = node.data();
        if(children.size() > 0 ) checkNode = node->getLastChild();
        if(!checkNode) continue;
        int key = checkNode->mDefRect->rectnumber();
        QList<TargetNode*> &list = counterNode[key];
        //检查有没有方向相同的目标
        bool found = false;
        foreach (TargetNode* tmpNode, list)
        {
            double cog_difff = checkNode->mDefRect->cog() - tmpNode->mDefRect->cog();
            if(fabs(cog_difff) >= 10) continue;
            //相同，删除冗余的目标（路径最短的目标）
            found = true;
            TargetNode* chk_parent = checkNode->topNode();
            TargetNode* tmp_parent = tmpNode->topNode();
            if(chk_parent && tmp_parent)
            {
                if(chk_parent->getDepth() < tmp_parent->getDepth())
                {
                    deleteNodeList.append(chk_parent->mSerialNum);
                } else
                {
                    deleteNodeList.append(tmp_parent->mSerialNum);
                }
            }
            break;
        }
        if(!found)
        {
            counterNode[key].append(checkNode);
        }
    }
    //删除相同运动状态的目标
    foreach (int key, deleteNodeList) {
        mTargetNodeMap.remove(key);
    }

    elapsed = t.elapsed();
    if(track_debug) qDebug()<<metaObject()->className()<<__FUNCTION__<<" check same status and merge elaped:"<<elapsed;
    t.start();

    //检查是否要对当次没有更新的目标进行预推更新
    if(mSettings.prediction_enabled)
    {
        int i= 1;
        foreach (QSharedPointer<TargetNode> node, mTargetNodeMap)
        {
//            qDebug()<<"start check make prediction"<<i++;
            //只对运动目标进行预推
            if(!node || !node->isNodeMoving()) continue;
            QList<uint> list = node->getVideoIndexList();
            if(list.size() == 0) continue;
            if(cur_cycle_index == list.last())
            {
                //目标本次进行了更新，则清楚目标原来的预推参数信息
                node->clearPrediction();
                continue;
            }
            //目标本次没有更新，需要进行检查本次是否需要预推
            uint baseIndex = list.last();
            if(node->mPredictionTimes > 0)
            {
                baseIndex = node->mPredictionIndex;
            }

            if(cur_cycle_index - baseIndex  < mTargetPredictionInterval) continue;
            //开始执行目标预推
            QTime test;
            test.start();
//            qDebug()<<"start make prediction";
            node->makePrediction(cur_cycle_index, task.first().updatetime());
//            qDebug()<<"make prediction end:"<<test.elapsed();
        }
    }

    elapsed = t.elapsed();
    if(track_debug) qDebug()<<"exe obj prediction elaped:"<<elapsed <<mSettings.prediction_enabled<<mSettings.check_target_gap;
    t.start();

    //删除很久没有更新的目标点
    deleteExpiredNode();
    //现在将目标进行输出
    outputTargets();

    elapsed = t.elapsed();
    if(track_debug) qDebug()<<" out put elaped:"<<elapsed;
    t.start();
}

NodeStatus zchxRadarTargetTrack::checkRoutePathSts(QList<TargetNode*>& newRoueNodeList, const QList<TargetNode*>& path)
{
    //只有一个节点不能确定状态
    if(path.size() < mSettings.confirm_target_cnt) return Node_UnDef;
    //检查对应的目标是否是方向统一
    QList<int> dir_change_index_list;
    for(int i=2; i<path.size(); i++)
    {
        double pre_cog = path[i-1]->mDefRect->cog();
        double cur_cog = path[i]->mDefRect->cog();
        if(isDirectionChange(pre_cog, cur_cog))
        {
            dir_change_index_list.append(i-1);
        }
    }
    if(dir_change_index_list.size() == 0)
    {
        //这里再检查每一段路径的长度是否符合要求。每一段路径的长度是否都差不多。如果差异较大，就从差异较大的节点开始抽出节点重新组成一个新的目标
        double pre_distance = -1.0;
        bool abnormal = false;
        if(mSettings.check_target_gap)
        {
            for(int i=2; i<path.size(); i++)
            {
                com::zhichenhaixin::proto::Latlon pre_center = path[i-1]->mDefRect->center();
                com::zhichenhaixin::proto::Latlon cur_center = path[i]->mDefRect->center();
                double distance = QGeoCoordinate(pre_center.latitude(), pre_center.longitude()).distanceTo(QGeoCoordinate(cur_center.latitude(), cur_center.longitude()));
                if(pre_distance < 0)
                {
                    pre_distance = distance;
                } else if(distance > 2.0 * pre_distance || distance > 0.5 * pre_distance)
                {
                    abnormal = true;
                }
                if(abnormal)
                {
                    newRoueNodeList.append(path[i]);
                }

            }
        }
        if(!abnormal)   return Node_Moving;
        return Node_GAP_ABNORMAL;
    }
    //从最后方向开始变化的点截断，让那个点作为目标最新的起点
    int start_index = dir_change_index_list.last();
    for(int i=start_index; i<path.size(); i++)
    {
        newRoueNodeList.append(path[i]);
    }
    return Node_UnDef;
}

QList<AreaNodeTable> zchxRadarTargetTrack::calculateTargetTrackMode(double max_speed, quint32 now, double scan_time)
{
    QList<AreaNodeTable> areaNodeTableList;
    if(mTargetNodeMap.size() == 0) return areaNodeTableList;

    QTime t;
    t.start();
    qDebug()<<"start calculate node's prediction area. and now current node size:"<<mTargetNodeMap.size();

    //通过计算目标的预推区域来计算区域是否存在相交，追越， 平行相遇等模型
    QList<PredictionNode> result_list;
    //1)先计算预推区域
    foreach (QSharedPointer<TargetNode> top_node, mTargetNodeMap)
    {
        if(!top_node) continue;

        //检查目标是否已经存在子类末端节点
        QList<TargetNode*> node_list = top_node->getAllBranchLastChild();
        //不存在子节点的情况，直接添加顶层节点
        if(node_list.size() == 0) node_list.append(top_node.data());
        int length = 0;
        foreach (TargetNode* node, node_list)
        {
            double old_speed = node->mDefRect->sogms();
            double old_lat = node->mDefRect->center().latitude();
            double old_lon = node->mDefRect->center().longitude();
            double old_cog = node->mDefRect->cog();
            double distance = old_speed * (now - top_node->mUpdateTime);
            bool point_node = false;
            if(node->mDefRect->sogms() < 0.1 && node->mStatus == Node_UnDef)
            {
                distance = max_speed * scan_time;
                point_node = true;
            }

            zchxTargetPrediction *prediction = 0;
            if(point_node)
            {
                //目标静止,目标的预推区域就是周围的圆形区域
                prediction = new zchxTargetPrediction(old_lat, old_lon, distance);
            } else
            {
                //目标运动，就在他的运动方向上构造区域
                QGeoCoordinate est_geo = QGeoCoordinate(old_lat, old_lon).atDistanceAndAzimuth(distance, old_cog);
                double est_lat = est_geo.latitude();
                double est_lon = est_geo.longitude();
                prediction = new zchxTargetPrediction(old_lat, old_lon, est_lat, est_lon, mSettings.prediction_width, 0.2);
            }
            if(prediction && prediction->isValid())
            {
                //更新节点对应的预推区域
                node->mDefRect->clear_prediction();
                QList<Latlon> area = prediction->getPredictionAreaLL();
                foreach (Latlon ll, area) {
                    com::zhichenhaixin::proto::Latlon* block = node->mDefRect->mutable_prediction()->add_area();
                    block->set_latitude(ll.lat);
                    block->set_longitude(ll.lon);
                }
                PredictionNode res;
                res.mNode = node;
                res.mPrediction = prediction;
                result_list.append(res);
                length++;
            }
        }
        //检查每一个节点是否都已经更新了预推区域
        if(length == 0)
        {
            //节点没有更新预推区域，这是异常情况
            if(DEBUG_TRACK_INFO) qDebug()<<"node:"<<top_node->mSerialNum<<" has no prediction area. abnormal occoured...";
        }
    }
    //开始计算目标预推区域和节点的对应关系
    //对应相交追越相遇模型计算他们的共同区域，并且抽出来，区域被重新分割，然后进行重新赋值
    for(int i=0; i<result_list.size(); i++)
    {
        PredictionNode cur = result_list[i];
        //分别计算相交 追越  相遇的预推区域
        bool found = false;
        QGeoCoordinate cur_geo(cur.mNode->mDefRect->center().latitude(), cur.mNode->mDefRect->center().longitude());
        for(int k=i+1; k<result_list.size(); k++)
        {
            if(i==k) continue;
            PredictionNode next = result_list[k];
            //先计算两者之间的实际距离值，距离太远(100m)，就不考虑二者的位置关系
            QGeoCoordinate next_geo(next.mNode->mDefRect->center().latitude(), next.mNode->mDefRect->center().longitude());
            double distance = cur_geo.distanceTo(next_geo);
            if(distance >= 100) continue;
            //相交
            QPolygonF andPoly = cur.mPrediction->getPredictionAreaMC().intersected(next.mPrediction->getPredictionAreaMC());
            if(andPoly.size() > 0)
            {
                found = true;
                AreaNodeTable table;
                table.mType = "AND";
                table.mArea = andPoly;
                table.mNodeList.append(cur.mNode);
                table.mNodeList.append(next.mNode);
                areaNodeTableList.append(table);
                QPolygonF subPoly = cur.mPrediction->getPredictionAreaMC().subtracted(next.mPrediction->getPredictionAreaMC());
                if(subPoly.size() > 0)
                {
                    table.mNodeList.clear();
                    table.mType = "SUB";
                    table.mArea = subPoly;
                    table.mNodeList.append(cur.mNode);
                    areaNodeTableList.append(table);
                }
                continue;
            }
            //追越或者相遇。计算二者运动方向的夹角，如果是小于5度，将预推区域旋转到同一方向，如果很近或者相交，则满足条件
            double sub_angle = cur.mNode->mDefRect->cog() - next.mNode->mDefRect->cog();
            //计算两个区域的或作为共同的区域
            QPolygonF unitedPoly = cur.mPrediction->getPredictionAreaMC().united(next.mPrediction->getPredictionAreaMC());
            //同向的判断 二者的角度差不多大小，就是相差5度  如果一个在0度左边，一个在0度右边 就是相差355
            if(fabs(sub_angle) <= 5.0 || fabs(sub_angle) >= 355.0)
            {
                AreaNodeTable table;
                table.mType = "OR";
                table.mArea = unitedPoly;
                table.mNodeList.append(cur.mNode);
                table.mNodeList.append(next.mNode);
                areaNodeTableList.append(table);
                found = true;

            } else
            {
                //检查是否是反向
                if(sub_angle < 0) sub_angle += 360.0;
                sub_angle -= 180.0;
                if(fabs(sub_angle) <= 5.0)
                {
                    AreaNodeTable table;
                    table.mType = "OR";
                    table.mArea = unitedPoly;
                    table.mNodeList.append(cur.mNode);
                    table.mNodeList.append(next.mNode);
                    areaNodeTableList.append(table);
                    found = true;
                }
            }


        }

        if(!found)
        {
            AreaNodeTable table;
            table.mType = "SELF";
            table.mArea = cur.mPrediction->getPredictionAreaMC();
            table.mNodeList.append(cur.mNode);
            areaNodeTableList.append(table);
        }

        if(cur.mPrediction)
        {
            delete cur.mPrediction;
            cur.mPrediction = 0;
        }
    }

    qDebug()<<"end calculate node's prediction area with table size:"<<areaNodeTableList.size()<<" time:"<<t.elapsed();

    if(DEBUG_TRACK_INFO)
    {
        //打印预推区域和节点
        qDebug()<<"start printf prediction area related node;";
        for(int i=0; i<areaNodeTableList.size(); i++)
        {
            AreaNodeTable table = areaNodeTableList[i];
            QStringList node_number_list;
            foreach (TargetNode* node, table.mNodeList) {
                node_number_list.append(QString::number(node->mSerialNum));
            }
            qDebug()<<"prediction["<<i+1<<"] type"<<table.mType<<" has node list:"<<node_number_list.join(" ");
        }
    }
    return areaNodeTableList;
}

void zchxRadarTargetTrack::splitAllRoutesIntoTargets(TargetNode *node, TargetNode *routeNode)
{
    if(DEBUG_TRACK_INFO) zchxTimeElapsedCounter counter(__FUNCTION__);
    if(!node || !routeNode) return;
    //目标确认,将路径进行分离,确认的路径保留,未确认的路径移除并作为单独的目标再次加入,等待下一个周期到来时进行继续更新
    for(int i=0; i<node->mChildren.size();)
    {
        //取得路径的第一个节点
        if(DEBUG_TRACK_INFO)    qDebug()<<"i = "<<i<<node->mChildren.size();
        TargetNode *child_lvl1 = node->mChildren[i].data();
        if(!child_lvl1) continue;
        if(child_lvl1 == routeNode)
        {
            //路径点保留
            i++;
            continue;
        }
        //将节点移除
        QSharedPointer<TargetNode> topNode = node->mChildren.takeAt(i);
        if(topNode)
        {
            topNode->mStatus = Node_UnDef;
            topNode->mParent = 0;
            int node_num = getCurrentNodeNum();
            topNode->mSerialNum = node_num;
            topNode->mPredictionNode = 0;
            topNode->clearPrediction();
            mTargetNodeMap.insert(node_num, topNode);
        }
    }
}

void zchxRadarTargetTrack::deleteExpiredNode()
{
    //清理目标,删除超时未更新的目标或者预推次数太多的目标
    quint32 time_of_day = QDateTime::currentDateTime().toTime_t();
    QList<int> allKeys = mTargetNodeMap.keys();
    foreach (int key, allKeys) {
        QSharedPointer<TargetNode> node = mTargetNodeMap[key];
        if(!node) continue;
        quint32 node_time = node->mUpdateTime;
        quint32 delta_time = time_of_day - node_time;
        if(delta_time >= mSettings.clear_target_time)
        {
            if(DEBUG_TRACK_INFO) qDebug()<<"remove node:"<<node.data()->mSerialNum<<"now:"<<time_of_day<<" node time:"<<node_time<<" delta_time:"<<delta_time<<" clear time:"<<mSettings.clear_target_time;
            mTargetNodeMap.remove(key);
            continue;
        }
    }
}

void zchxRadarTargetTrack::updateTrackPointWithNode(zchxRadarSurfaceTrack& list, TargetNode *node, int* silent_num)
{
    if(!node) return;
    TargetNode *child = node->getLastChild();
    if(node->mStatus == Node_Moving && node->mPredictionNode)
    {
        child = node->mPredictionNode;
    }
    if(!child) return;
    //确认目标是否输出
    if(node->isNodeSilent())
    {
        if(node->mFalseAlarm) return;
        if(!mSettings.output_point) return;
        if(silent_num) *silent_num = (*silent_num) + 1;
    } else if(node->isNodeMoving())
    {
        if(child->mDefRect->sogms() < mSettings.output_target_min_speed) return;
    } else
    {
        //目标状态未确定
        return;
    }


    int node_number = node->mSerialNum;
    zchxTrackPoint *trackObj = list.add_trackpoints();
    if(!trackObj) return;
    zchxRadarRectDef *target = child->mDefRect;

    //编号
    trackObj->set_radarsiteid(QString("%1_%2").arg(mSettings.radar_id).arg(mSettings.channel_id).toUtf8().data());
    trackObj->set_tracknumber(node_number);
    trackObj->set_trackconfirmed(node->mStatus == Node_Moving? true : false);
    trackObj->set_objtype(1);
    //当前目标
    trackObj->mutable_current()->CopyFrom(*target);
    trackObj->mutable_current()->set_sogknot(target->sogms() * 3.6 / 1.852);  //输出的速度为节
    //历史轨迹输出
    if(node->isNodeMoving())
    {
        //历史轨迹数据按照时间顺序进行输出，时间越早则靠后。top节点在最后面
        TargetNode* cur = node->getLastChild();
        while (cur) {
            zchxRadarRectDef *history = trackObj->add_tracks();
            history->CopyFrom(*(cur->mDefRect));
            cur = cur->mParent;
        }
    }
}

void zchxRadarTargetTrack::updateRectMapWithNode(zchxRadarRectMap &map, TargetNode *node)
{
    if(!node) return;

    //开始更新余晖数据
    zchxRadarRect rect;
    rect.set_node_num(node->mSerialNum);
    rect.set_cur_est_count(/*node->est_count*/0);
    rect.set_dir_confirmed(node->mStatus == Node_Moving);
    if(node->mStatus == Node_Moving)
    {
        //需要更新历史数据
        TargetNode * latest_node = node->getLastChild();
        if(!latest_node) return;
        //当前的矩形数据
        rect.mutable_current_rect()->CopyFrom(*(latest_node->mDefRect));
//        rect.mutable_current_rect()->set_rectnumber(rect_num);
        //历史的矩形数据
        TargetNode* work_node = node;
        QList<TargetNode*> route_list;      //遍历的路径
        while (work_node) {
            route_list.prepend(work_node); //从后面开始添加路径,时间越靠近的在最前面
            if(work_node == latest_node) break;
            if(work_node->mChildren.size())
            {
                work_node = work_node->mChildren.first().data();
            } else
            {
                break;
            }
        }
        for(int i=0; i<route_list.size(); i++)
        {
            zchxRadarRectDef *his_rect = rect.add_history_rect_list();
            his_rect->CopyFrom(*(route_list[i]->mDefRect));
            if(rect.history_rect_list_size() == 20) break;
        }
    } else
    {
        //不需要更新历史数据,只更新目标的最初数据
        rect.mutable_current_rect()->CopyFrom(*(node->getLastChild()->mDefRect));
#if 0
        // 这里便于调试,将所有的节点的预推区域也加上
        for(int i=0; i<node->children.size(); i++)
        {
            TargetNode *worknode = node->children[i].data();
            while (worknode)
            {
                if(worknode->rect->has_predictionareas())
                {
                    rect.mutable_current_rect()->mutable_predictionareas()->CopyFrom(worknode->rect->predictionareas());
                }
                if(worknode->children.size() == 0) break;
                worknode = worknode->children[0].data();
            }
        }
#endif
    }
    map[node->mSerialNum] = rect;
}

void zchxRadarTargetTrack::outputRoutePath()
{
    return;
    static qint64 count = 0;
    zchxRadarRouteNodes nodeList;
    //遍历目标进行数据组装
    for(TargetNodeMap::iterator it = mTargetNodeMap.begin(); it != mTargetNodeMap.end(); it++)
    {
        TargetNode *node = it->data();
        if(!node) continue;
        com::zhichenhaixin::proto::RouteNode* route = nodeList.add_node_list();
        route->set_node_num(node->mSerialNum);
        route->mutable_top_node()->CopyFrom(*(node->mDefRect));
        for(int i=0; i<node->mChildren.size();i++)
        {
            TargetNode* child = node->mChildren.at(i).data();
            com::zhichenhaixin::proto::RoutePath *path = route->add_path_list();
            path->add_path()->CopyFrom(*(child->mDefRect));
            while (child->mChildren.size() > 0) {
                child = child->mChildren[0].data();
                path->add_path()->CopyFrom(*(child->mDefRect));
            }
        }
    }
    if(nodeList.node_list_size() > 0)
    {
        emit signalSendRoutePath(nodeList);
    }
    count++;
    if(count >= 10000) count = 10000;
}

void zchxRadarTargetTrack::outputTargets()
{
    //遍历目标进行数据组装
    QList<TargetNode*> output_list;
    for(TargetNodeMap::iterator it = mTargetNodeMap.begin(); it != mTargetNodeMap.end(); it++)
    {
        TargetNode *node = it->data();
        if(!node) continue;
        zchxRadarRectDef *top_rect = node->mDefRect;
        if(!top_rect) continue;
        if(node->isOutputEnabled()) output_list.append(node);
    }
    if(output_list.size() == 0) return;


    zchxRadarSurfaceTrack   track_list;         //雷达目标数据输出
    track_list.set_flag(1);
    track_list.set_sourceid("240");
    track_list.set_utc(QDateTime::currentMSecsSinceEpoch());
    //遍历目标进行数据组装
    int silent_obj_count = 0;
    int total_size = output_list.size();
    foreach (TargetNode *node, output_list)
    {
        if(!node) continue;
        zchxRadarRectDef *top_rect = node->mDefRect;
        if(!top_rect) continue;
        //静止点饥数据输出控制
        if(node->isNodeSilent() && silent_obj_count > 0 && silent_obj_count > total_size * 0.4) continue;
        //构造目标数据
        updateTrackPointWithNode(track_list, node, &silent_obj_count);
    }
    track_list.set_length(track_list.trackpoints_size());

    if(track_list.trackpoints_size())
    {
        emit signalSendTracks(track_list);
    }
}

void zchxRadarTargetTrack::appendNode(TargetNode *node, int source)
{
    if(!node) return;
    node->mStatus = Node_UnDef;
    node->mSerialNum = getCurrentNodeNum();
    node->mParent = 0;
    node->mPredictionNode = 0;
    node->clearPrediction();
    if(source == 0)
    {
        if(DEBUG_TRACK_INFO)qDebug()<<"make new now from orignal rect:"<<node<<node->mDefRect->rectnumber()<<node->mSerialNum;
    } else
    {
        if(DEBUG_TRACK_INFO)qDebug()<<"make new now from old ununsed route rect:"<<node<<node->mDefRect->rectnumber()<<node->mSerialNum;
    }
    mTargetNodeMap.insert(node->mSerialNum, QSharedPointer<TargetNode>(node));
    if(track_debug) qDebug()<<"new node maked now:"<<node->mSerialNum<<node->mUpdateTime;
}

int zchxRadarTargetTrack::getCurrentNodeNum()
{
    //更新编号,没有找到重复的点,直接添加
    if(mRectNum > mMaxNum)
    {
        mRectNum = mMinNum;
    }

    //检查当前是否还存在空闲的
    while (mRectNum <= mMaxNum) {
        if(mTargetNodeMap.contains(mRectNum)){
            mRectNum++;
            continue;
        }
        break;
    }

    return mRectNum;
}

void zchxRadarTargetTrack::process(const zchxRadarTrackTask &task)
{
    zchxTimeElapsedCounter counter(QString(metaObject()->className()) + " : " + QString(__FUNCTION__));
    processWithPossibleRoute(task);
    return;
}

void zchxRadarTargetTrack::dumpTargetDistance(const QString &tag, double merge_dis)
{
    if(track_debug) qDebug()<<"dump target distance now:"<<tag;
    QList<int> keys = mRadarRectMap.keys();
    for(int i=0; i<keys.size(); i++)
    {
        zchxRadarRect cur = mRadarRectMap[keys[i]];
        double cur_lat = cur.current_rect().center().latitude();
        double cur_lon = cur.current_rect().center().longitude();
        for(int k = i+1; k<keys.size(); k++)
        {
            zchxRadarRect next = mRadarRectMap[keys[k]];
            double next_lat = next.current_rect().center().latitude();
            double next_lon = next.current_rect().center().longitude();
            double distance = getDisDeg(cur_lat, cur_lon, next_lat, next_lon);
            if(distance < merge_dis)
            {
                if(track_debug) qDebug()<<"found abormal targets:"<<keys[i]<<keys[k]<<"  distance:"<<distance<<" time:"<<cur.current_rect().updatetime()<<next.current_rect().updatetime();
            }
        }
    }
}

void zchxRadarTargetTrack::checkTargetRectAfterUpdate(double merge_dis)
{
    QList<int> keys = mRadarRectMap.keys();
    for(int i=0; i<keys.size(); i++)
    {
        if(!mRadarRectMap.contains(keys[i])) continue;
        zchxRadarRect cur = mRadarRectMap[keys[i]];
        double cur_lat = cur.current_rect().center().latitude();
        double cur_lon = cur.current_rect().center().longitude();
        for(int k = i+1; k<keys.size(); k++)
        {
            if(!mRadarRectMap.contains(keys[k])) continue;
            zchxRadarRect next = mRadarRectMap[keys[k]];
            double next_lat = next.current_rect().center().latitude();
            double next_lon = next.current_rect().center().longitude();
            double distance = getDisDeg(cur_lat, cur_lon, next_lat, next_lon);
            if(distance < merge_dis)
            {
                if(cur.dir_confirmed() ^ next.dir_confirmed())
                {
                    if(track_debug) qDebug()<<"found abormal targets:"<<keys[i]<<keys[k]<<"  distance:"<<distance<<" confirmed:"<<cur.dir_confirmed()<<next.dir_confirmed()<<" remove not confirmed one.";

                    if(cur.dir_confirmed() && !next.dir_confirmed())
                    {
                        mRadarRectMap.remove(keys[k]);
                        continue;
                    }
                    if((!cur.dir_confirmed()) && next.dir_confirmed())
                    {
                        mRadarRectMap.remove(keys[i]);
                        break;
                    }
                } else
                {
                    if(track_debug) qDebug()<<"found abormal targets:"<<keys[i]<<keys[k]<<"  distance:"<<distance<<" time:"<<cur.current_rect().updatetime()<<next.current_rect().updatetime()<<" remove old one.";

                    if(next.current_rect().updatetime() <= cur.current_rect().updatetime())
                    {
                        mRadarRectMap.remove(keys[k]);
                    } else
                    {
                        mRadarRectMap.remove(keys[i]);
                        break;
                    }
                }


            }
        }
    }
}


void zchxRadarTargetTrack::appendUserDefObj(const zchxCommon::UserSpecifiedObj &obj)
{
    if(obj.mTrackNum <= 0) return;
    mUserDefObj[obj.mTrackNum] = obj;
}

void zchxRadarTargetTrack::removeUserDefObj(const zchxCommon::UserSpecifiedObj &obj)
{
    mUserDefObj.remove(obj.mTrackNum);
}

void zchxRadarTargetTrack::updateParseSetting(const zchxVideoParserSettings &setting)
{
    mSettings = setting;
//    mTargetNodeMap.clear();
}
