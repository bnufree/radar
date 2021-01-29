#include "zchxrecvutils.h"
#include "zmq.h"
#include "zchxdatadef.h"
#include "ZCHXRadarDataDef.pb.h"
#include "zchxcommonutils.h"

using namespace com::zhichenhaixin::proto;
ZCHXReceiverThread::ZCHXReceiverThread(const QString& host, int port, const QList<DataRecvPair>& cmd_list, QObject *parent)
    :mIsOver(false)
    ,mCtx(0)
    ,mSocket(0)
    ,mIsConnect(false)
    ,mCmdList(cmd_list)
    ,QThread(parent)
    ,mHost(host)
    ,mPort(port)
{
    mCtx = zmq_ctx_new();
    mUrl = QString("tcp://%1:%2").arg(mHost).arg(mPort);
    foreach (DataRecvPair pair, mCmdList) {
        mTopicList.append(pair.topic);
    }
}

ZCHXReceiverThread::~ZCHXReceiverThread()
{
    disconnectToHost();
    if(mCtx) zmq_ctx_destroy(mCtx);
}

void ZCHXReceiverThread::disconnectToHost()
{
    if(mSocket)
    {
        qDebug()<<"data recv has been canceled:"<<mPort<<mTopicList;
        zmq_close(mSocket);
    }
    mSocket = 0;
}

bool ZCHXReceiverThread::connectToHost()
{
    disconnectToHost();
    if((!mCtx) && ((mCtx = zmq_ctx_new()) == NULL)) return false;
    if((mSocket = zmq_socket(mCtx, ZMQ_SUB)) == NULL) return false;

    //设置topic 过滤
    if(mTopicList.size() > 0)
    foreach (QString topic, mTopicList) {
        const char *filter = zchxCommonUtils::QString2Char(topic);
        if(zmq_setsockopt(mSocket, ZMQ_SUBSCRIBE, filter, strlen(filter))) return false;
    } else
    {
        zmq_setsockopt(mSocket, ZMQ_SUBSCRIBE, "", 0);
    }
    //设置等待时间
    int timeout = 5 * 1000; //5s超时限制，没有收到消息就退出
    if(zmq_setsockopt(mSocket, ZMQ_RCVTIMEO, &timeout, sizeof(timeout))) return false;
    timeout = 0;
    zmq_setsockopt(mSocket, ZMQ_LINGER, &timeout, sizeof(timeout));
    //开始连接
    const char* sAddress = zchxCommonUtils::QString2Char(mUrl);
    int sts = zmq_connect(mSocket, sAddress);
    if(sts != 0)
    {
        qDebug()<<"connect to server failed .code:"<<zmq_errno()<<" str:"<<zmq_strerror(zmq_errno())<<sAddress;
    } else
    {
        qDebug()<<"connect to server succeed."<<sAddress<<mTopicList;
    }


    delete []sAddress;

    return sts == 0;

}

void ZCHXReceiverThread::run()
{
    int no_recv_num = 0;
    while(1)
    {
        if(mIsOver)
        {
            disconnectToHost();
            mIsConnect = false;
            break;
        } else
        {
            //检查是否需要重连
            if(!mIsConnect)
            {
                QString url;
                url.sprintf("tcp://%s:%s", mHost.toStdString().data(), QString::number(mPort).toStdString().data());
//                qDebug()<<"start connect to server:"<<url<<" topic:"<<mTopicList;
                //开始连接服务器
                if(!connectToHost())
                {
                    url.append(" ");
                    url.append(QString::fromStdString(zmq_strerror(zmq_errno())));
                    emit signalConnectedStatus(false, url, mTopicList.join(","));
                    sleep(3);
                    continue;
                } else
                {
                    mIsConnect = true;
                    emit signalConnectedStatus(true, url, mTopicList.join(","));
                }
            }
            //开始接收数据,数据有可能多帧发送
            QByteArrayList recvlist;
            int length = 0;
            while (1) {
                int64_t more = 0;
                size_t more_size = sizeof (more);
                zmq_msg_t msg;
                zmq_msg_init(&msg);
                zmq_recvmsg(mSocket, &msg, 0);
                QByteArray bytes2 = QByteArray((char*)zmq_msg_data(&msg),(int)(zmq_msg_size(&msg)));
                zmq_msg_close(&msg);
                if(bytes2.length() > 0)
                {
                    recvlist.append(bytes2);
                }
                length += bytes2.length();
                zmq_getsockopt (mSocket, ZMQ_RCVMORE, &more, &more_size);
                if (more == 0)      //判断是否是最后消息
                {
                    break; // 已到达最后一帧
                }
                //msleep(1000);
            }
            if(recvlist.length() == 0)
            {
//                qDebug()<<"no data received:"<<mTopicList<<QDateTime::currentDateTime()<<mUrl;
                //没有接收到数据
                no_recv_num++;
                if(no_recv_num >= 10)
                {
                    //这里清除掉旧的记录,自动重连
                    mIsConnect = false;
                }
                continue;
            } else
            {
                no_recv_num = 0;
            }
//            qDebug()<<"recv data tag:"<<recvlist[0]<<QDateTime::fromMSecsSinceEpoch(recvlist[1].toLongLong()).toString("hh:mm:ss")<<QDateTime::currentDateTime().toString("hh:mm:ss")<<" data pakage size:"<<recvlist.size();
            parseRecvData(recvlist);
        }
    }
}

void ZCHXReceiverThread::parseRecvData(const QByteArrayList &list)
{
    if(list.size() < 2) return;
    QString topic = QString::fromStdString(list.first().data());
    DataRecvPair pair;
    pair.topic = topic;
    int index = mCmdList.indexOf(pair);
    if(index == -1) return;
    int cmd = mCmdList[index].data_id;
    switch (cmd) {
    case zchxCommon::THREAD_DATA_AIS:
        parseAis(list.last());
        break;
//    case zchxCommon::THREAD_DATA_FILTER:
//        parseRadarFilterArea(list.last());
//        break;
    case zchxCommon::THREAD_DATA_RADAR_REPORT:
        parseRadarReport(list.last());
        break;
    case zchxCommon::THREAD_DATA_RADAR_VIDEO:
        parseRadarVideo(list.last());
        break;
    case zchxCommon::THREAD_DATA_RADAR_TRACK:
        parseRadarTrack(list.last());
        break;
    case zchxCommon::THREAD_DATA_SVRCFG:
        parseSvrCfg(list.last());
        break;
    case zchxCommon::THREAD_DATA_DEL_NODE_LOG:
        parseRadarNodeLog(list.last());
        break;
    default:
        break;
    }

}

void ZCHXReceiverThread::parseAis(const QByteArray &bytes)
{

}

void transferNodeRect(ZCHX::Data::ITF_RadarRectDef &out, const RadarRectDef &in)
{
    out.rectNumber = in.rectnumber();
    out.center.lat = in.center().latitude();
    out.center.lon = in.center().longitude();
    out.updateTime = in.updatetime();
    out.isRealData = in.realdata();
    out.cog = in.cog();
    out.sogKnot = in.sogknot();
    out.sogMps = in.sogms();
    for(int i=0; i<in.outline_size(); i++)
    {
        out.outline.append(ZCHX::Data::LatLon(in.outline(i).latitude(), in.outline(i).longitude()));
    }
    out.boundRect.bottomRight.lat = in.boundrect().bottomright().latitude();
    out.boundRect.bottomRight.lon = in.boundrect().bottomright().longitude();
    out.boundRect.topLeft.lat = in.boundrect().topleft().latitude();
    out.boundRect.topLeft.lon = in.boundrect().topleft().longitude();
    out.boundRect.diameter = in.boundrect().diameter();
    out.maxSeg.start.lat = in.seg().start().latitude();
    out.maxSeg.start.lon = in.seg().start().longitude();
    out.maxSeg.end.lat = in.seg().end().latitude();
    out.maxSeg.end.lon = in.seg().end().longitude();
    out.referWidth = in.fixedimg().width();
    out.referHeight = in.fixedimg().height();
    for(int i=0; i<in.fixedimg().points_size(); i++)
    {
        out.pixPoints.append(QPoint(in.fixedimg().points(i).x(), in.fixedimg().points(i).y()));
    }

    //添加预推区域
    if(in.has_prediction())
    {
        com::zhichenhaixin::proto::PredictionArea area(in.prediction());
        for(int m =0; m<area.area_size(); m++)
        {
            ZCHX::Data::LatLon block;
            block.lat = area.area(m).latitude();
            block.lon = area.area(m).longitude();
            out.predictionArea.append(block);
        }
    }
}

void transferTrack2Point(const RadarSurfaceTrack &objRadarSurfaceTrack,
                         QMap<int, QList<ZCHX::Data::ITF_RadarPoint> >& radarPointList)
{
    int size = objRadarSurfaceTrack.trackpoints_size();

    for (int i = 0; i < size; i++)
    {
        const TrackPoint &point = objRadarSurfaceTrack.trackpoints(i);
        ZCHX::Data::ITF_RadarPoint item;
        item.warnStatus = 0;
        item.radarSiteID                = QString::fromStdString(point.radarsiteid());
        item.trackNumber                = point.tracknumber();
        transferNodeRect(item.currentRect, point.current());
        for(int k=0; k<point.tracks_size(); k++)
        {
            ZCHX::Data::ITF_RadarRectDef temp;
            transferNodeRect(temp, point.tracks(k));
            item.historyRects.append(temp);
        }
        item.directionConfirmed = point.trackconfirmed();
        if(!point.has_objtype())
        {
            item.objType = ZCHX::Data::RadarPointNormal;
        } else
        {
            item.objType = point.objtype();
        }
        if(point.has_objname()) item.objName = QString::fromStdString(point.objname());
        item.diameter = item.currentRect.boundRect.diameter;

        radarPointList[item.radarSiteID.split("_").at(0).toInt()].append(item);
    }
}

void ZCHXReceiverThread::parseRadarTrack(const QByteArray &bytes)
{
    RadarSurfaceTrack objRadarSurfaceTrack;
    QMap<int, QList<ZCHX::Data::ITF_RadarPoint>> radarPointList;
    if(!objRadarSurfaceTrack.ParseFromArray(bytes.data(), bytes.size())) return;
    transferTrack2Point(objRadarSurfaceTrack, radarPointList);
    emit signalSendRadarPoint(radarPointList);
}

void ZCHXReceiverThread::parseRadarNodeLog(const QByteArray &bytes)
{
    QJsonDocument doc = QJsonDocument::fromJson(bytes);
    if(!doc.isArray()) return;
    zchxCommon::zchxDelNodeLogList src(doc.array());
    QList<ZCHX::Data::ITF_RadarNodeLog> list;
    foreach (zchxCommon::zchxDelNodeLog log, src) {
        ZCHX::Data::ITF_RadarNodeLog data;
        data.lat = log.lat;
        data.lon = log.lon;
        data.reason = log.reason;
        data.time = log.time;
        data.track = log.track;
        list.append(data);
    }

    emit signalSendRadarNodeLog(list);
}


void ZCHXReceiverThread::parseRadarVideo(const QByteArray &bytes)
{
    RadarVideoImages result;
    if(!result.ParseFromArray(bytes.data(), bytes.size())) return;
    QList<ZCHX::Data::ITF_RadarVideoImage> list;
    for(int i=0; i<result.images_size(); i++)
    {
        RadarVideoImage src = result.images(i);
        ZCHX::Data::ITF_RadarVideoImage img;
        img.radar_id = QString::fromStdString(src.radarid());
        img.channel_id = src.channelid();
        img.lat = src.center().latitude();
        img.lon = src.center().longitude();
        img.distance = src.radius();
        img.image = QByteArray(src.imagedata().c_str(), src.imagedata().length());
        img.timestamp = src.utc();
        list.append(img);
    }

    emit signalSendRadarVideo(list);
}

void ZCHXReceiverThread::parseRadarFilterArea(const QByteArray &bytes)
{

}

void ZCHXReceiverThread::parseRadarReport(const QByteArray &bytes)
{
    qDebug()<<"recv report data:"<<bytes.size();
    QJsonDocument doc = QJsonDocument::fromJson(bytes);
    if(!doc.isArray()) return;
    QJsonArray array = doc.array();
    zchxCommon::zchxRadarReportList report_list(array);
    emit signalSendRadarReport(report_list);
}

void ZCHXReceiverThread::parseSvrCfg(const QByteArray &bytes)
{
    QJsonDocument doc = QJsonDocument::fromJson(bytes);
    if(!doc.isObject()) return;
    QJsonObject obj = doc.object();
    int cmd = obj.value(JSON_CMD).toInt();
    if(cmd == zchxCommon::SVR_UPDATE_RADAR_RADIUS)
    {
        QJsonArray array = obj.value(JSON_VAL).toArray();
        if(array.size() == 4)
        {
            int channel = array[0].toInt();
            double radius = array[1].toDouble();
            double factor = array[2].toDouble();
            int radar = array[3].toString().toInt();
            emit signalUpdateRadarChannelRadius(radar, channel, radius, factor);
        }
    } else if(cmd == zchxCommon::SVR_UPDATE_RADAR_FILTER)
    {
        zchxCommon::zchxfilterAreaList area_list(obj.value(JSON_VAL).toArray());
        QList<ZCHX::Data::ITF_IslandLine> filter_list;
        foreach (zchxCommon::zchxFilterArea filter, area_list) {
            ZCHX::Data::ITF_IslandLine line;
            line.name = filter.name;
            if(filter.type == 1)
            {
                line.warnColor = "#00ff00";
            } else
            {
                line.warnColor = "#ff0000";
            }
            foreach (zchxCommon::zchxLatlon ll, filter.area) {
                line.path.push_back(std::pair<double, double>(ll.lat, ll.lon));
            }
            line.id = filter.id;

            filter_list.append(line);
        }
        emit signalSendLimitDataList(filter_list);

    } else if(cmd == zchxCommon::SVR_UPDATE_PUBLISH_PORT_STATUS)
    {
        emit signalUpdatePublishPortStatus(zchxCommon::zchxPublishSettingsList(obj.value(JSON_VAL).toArray()));

    } else if(cmd == zchxCommon::SVR_UPDATE_RADAR_TYPE)
    {
        emit signalUpdateRadarType(zchxCommon::zchxRadarTypeData(obj.value(JSON_VAL).toObject()));

    } else if(cmd == zchxCommon::SVR_UPDATE_PUBLISH_SETTING)
    {
        emit signalSendNewPublishSetting(zchxCommon::zchxPublishSettingsList(obj.value(JSON_VAL).toArray()));
    } else if(cmd == zchxCommon::SVR_UPDATE_RADAR_DEV)
    {
        emit signalSendNewDevList(zchxCommon::zchxRadarDeviceList(obj.value(JSON_VAL).toArray()));
    }

}
