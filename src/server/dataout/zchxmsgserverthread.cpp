#include "zchxmsgserverthread.h"
#include <QDebug>
#include <QDateTime>
#include "zmq.h"
#include "zchxcfgmgr.h"

zchxMsgServerThread::zchxMsgServerThread(void* ctx, int port, QObject *parent)
    : zchxDataThread(ZMQ_REP, ctx, port, true, parent)
{
    mCfgMgr = 0;
    qRegisterMetaType<zchxCommon::zchxPortStatus>("const zchxCommon::zchxPortStatus&");
    qRegisterMetaType<zchxCommon::zchxfilterAreaList>("const zchxCommon::zchxfilterAreaList&");
    qRegisterMetaType<zchxCommon::zchxRadarCtrlDef>("const zchxCommon::zchxRadarCtrlDef&");
}

zchxMsgServerThread::~zchxMsgServerThread()
{
}

void zchxMsgServerThread::run()
{
    zchxCommon::zchxPortStatus sts;
    sts.port = getPort();
    sts.sts = isOK();
    sts.topic = "client req/rep";
    emit signalPortStatus(sts);
    if(!isOK())
    {
        qDebug()<<"socket init failed.";
        return;
    }
    while (!mStop)
    {
        //获取客户端发送的信息，客户端发送2段信息，时间,内容
        QByteArrayList recvlist;
        while (1) {
            int64_t more = 0;
            size_t more_size = sizeof(more);
            zmq_msg_t msg;
            zmq_msg_init(&msg);
            zmq_recvmsg(mSocket, &msg, 0);
            QByteArray bytes = QByteArray((char*)zmq_msg_data(&msg),(int)(zmq_msg_size(&msg)));
            zmq_msg_close(&msg);
            if(bytes.length() > 0)
            {
                recvlist.append(bytes);
            }
            zmq_getsockopt (mSocket, ZMQ_RCVMORE, &more, &more_size);
            if (more == 0) break;
        }
        if(recvlist.size()  == 2)
        {

            qint64 msg_time = recvlist[0].toLongLong();
            QJsonObject retObj;
            int cmd = zchxCommon::Msg_Undefined;
            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(recvlist[1], &error);
            if(error.error == QJsonParseError::NoError && doc.isObject())
            {
                QJsonObject obj = doc.object();
                cmd = (zchxCommon::MsgCmd)(obj.value(JSON_CMD).toInt());
                bool sts = false;
                switch (cmd)
                {
                //用户操作相关
                case zchxCommon::Msg_Edit_FilterArea:
                case zchxCommon::Msg_Delete_FilterArea:
                    sts = processFilterAreaMsg(cmd, obj.value(JSON_VAL));
                    break;
                case zchxCommon::Msg_Heart:
                    sts = true;
                    break;
                //用户登陆，请求配置信息
                case zchxCommon::Msg_Request_Cfg:
                    sts = processRequestCfg(retObj);
                    break;
                case zchxCommon::Msg_Update_PublishSetting:
                case zchxCommon::Msg_Update_ParseSetting:
                case zchxCommon::Msg_Update_ChannelSetting:
                case zchxCommon::Msg_Update_RadarBaseSetting:
                case zchxCommon::Msg_Delete_Radar:
                case zchxCommon::Msg_Update_BaseAndChannel:
                    sts = processUpdateCfg(cmd, obj.value(JSON_VAL));
                    break;
                case zchxCommon::Msg_Update_RadarCtrl:
                {
                    sts = true;
                    emit signalSendCtrlFromClient(zchxCommon::zchxRadarCtrlDef(obj.value(JSON_VAL).toObject()));
                    break;
                }

                default:
                    break;
                }

                retObj.insert(JSON_STATUS, sts ? JSON_OK : JSON_ERROR);
                if(!sts) retObj.insert(JSON_STATUS_STR, "contact server for more information");
                retObj.insert(JSON_CMD, cmd);
            } else
            {
                retObj.insert(JSON_STATUS, JSON_ERROR);
                retObj.insert(JSON_STATUS_STR, "parse json error");
                retObj.insert(JSON_CMD, cmd);
                retObj.insert(JSON_VAL, QString::fromUtf8(recvlist[1]));
            }
            retObj.insert(JSON_REQUEST_TIME, msg_time);

            QByteArray ret = QJsonDocument(retObj).toJson();
            QByteArray sTimeArray = QString::number(QDateTime::currentMSecsSinceEpoch()).toUtf8();
            zmq_send(mSocket, sTimeArray.data(), sTimeArray.size(), ZMQ_SNDMORE);
            zmq_send(mSocket, ret.data(), ret.size(), 0);
        }
        msleep(500);
    }

    if(mSocket)
    {
        zmq_close(mSocket);
    }
}

bool zchxMsgServerThread::processRequestCfg(QJsonObject &obj)
{
    if(mCfgMgr)
    {
        obj.insert(JSON_VAL, mCfgMgr->config()->toJson());
        return true;
    }

    return false;

}

bool zchxMsgServerThread::processUpdateCfg(int cmd, const QJsonValue& val)
{
    if(mCfgMgr)
    {
        mCfgMgr->updateCfg(cmd, val);
        return true;
    }

    return false;
}

bool zchxMsgServerThread::processFilterAreaMsg(int cmd, const QJsonValue &val)
{
    zchxCommon::zchxFilterArea area(val.toObject());
    emit signalSendEditFilterAreas(cmd, zchxCommon::zchxfilterAreaList(area));
    return true;
}
