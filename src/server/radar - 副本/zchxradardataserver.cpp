#include "zchxradardataserver.h"
#include "zchxmulticastdatasocket.h"
#include "zchxradarvideowinrarthread.h"
#include "zchxradarvideoparser.h"
#include <QDir>
#include "zchxcommonutils.h"
#include <QCoreApplication>

#ifdef Q_OS_UNIX
    typedef uint8_t     UINT8;
#endif

#define WINRAR_FILE_SIZE 20
#define VIDEO_FILE_DIR  "VideData"
#define REPORT_FILE_DIR "ReportData"
#define HEART_FILE_DIR   "HeartData"
#define OUTPUT_DATA_DIR "OutputData"
#define FILE_MAX_SIZE       30720
typedef uint8_t UINT8;


QNetworkInterface findNIF(const QString& ip)
{
    QNetworkInterface res;
    foreach(QNetworkInterface nif, QNetworkInterface::allInterfaces())
    {
        foreach (QNetworkAddressEntry entry, nif.addressEntries()) {
            if(entry.ip().toString() == ip)
            {
                res = nif;
                break;
            }
        }
    }
    return res;
}

zchxRadarDataController::zchxRadarDataController(const zchxCommon::zchxRadarChannel& channel,
                                                 const QString &local_ip,
                                                 QObject *parent)
    : QThread(parent)
    , mIsOver(false)
    , mLocalIP(local_ip)
{
    mLocalIPList = zchxCommonUtils::getAllIpv4List();
    mCurIPIndex = mLocalIPList.indexOf(mLocalIP);
    if(mCurIPIndex < 0) mCurIPIndex = 0;
    updateNewSetting(channel);
}

void zchxRadarDataController::writeData(const QByteArray &bytes, int type)
{
    mHeartDataList.append(bytes);
}
void zchxRadarDataController::updateNewSetting(const zchxCommon::zchxRadarChannel& channel)
{
    mHostChange = true;
    mVideoIP = channel.video.ip;
    mVideoPort = channel.video.port;
    mReportIP = channel.report.ip;
    mReportPort = channel.report.port;
    mHeartIP = channel.heart.ip;
    mHeartPort = channel.heart.port;
}

zchxRadarDataController::~zchxRadarDataController()
{
    stop();
}

void zchxRadarDataController::stop()
{
    setOver(true);
}


void zchxRadarDataController::run()
{
    if(mLocalIPList.size() == 0) return;
    qint64 last_init_time = 0;
    zchxMulticastDataScoket* heart_s = 0;
    zchxMulticastDataScoket* video_s = 0;
    zchxMulticastDataScoket* report_s = 0;
    while (!mIsOver)
    {
        if(mHostChange || !report_s || !heart_s || !video_s)
        {
            if(mCurIPIndex >= mLocalIPList.size()) mCurIPIndex = 0;
            while (mCurIPIndex < mLocalIPList.size())
            {
                if(heart_s) {delete heart_s; heart_s = 0;}
                if(video_s) {delete video_s; video_s = 0;}
                if(report_s) {delete report_s; report_s = 0; }

                mLocalIP = mLocalIPList[mCurIPIndex];
                mCurIPIndex = (mCurIPIndex + 1) % mLocalIPList.size();
                QNetworkInterface nif = findNIF(mLocalIP);
                //心跳
                heart_s = new zchxMulticastDataScoket(mLocalIP, mHeartIP, mHeartPort, nif);
//                if(!heart_s->isInit()) continue;
                connect(heart_s, SIGNAL(signalSendRecvData(QByteArray)), this, SIGNAL(signalSendHeartData(QByteArray)));
                //回波
                video_s = new zchxMulticastDataScoket(mLocalIP, mVideoIP, mVideoPort, nif);
//                if(!video_s->isInit()) continue;
                connect(video_s, SIGNAL(signalSendRecvData(QByteArray)), this, SIGNAL(signalSendVideoData(QByteArray)));
                //报告
                report_s = new zchxMulticastDataScoket(mLocalIP, mReportIP, mReportPort, nif);
//                if(!report_s->isInit()) continue;
                connect(report_s, SIGNAL(signalSendRecvData(QByteArray)), this, SIGNAL(signalSendReportData(QByteArray)));
                last_init_time = QDateTime::currentMSecsSinceEpoch();
                mHostChange = false;
                qDebug()<<"all socket init ok. start recv data now."<<mLocalIP;
                break;
            }
        }
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        if(last_init_time > 0 && now - last_init_time > 10000)
        {
//            if((!report_s->isFine()) ||(!video_s->isFine()))
//            {
//                qDebug()<<"no data recv. goto reinit socket."<<mLocalIP;
//                mHostChange = true;
//                continue;
//            }

            emit signalSendRecvIP(mLocalIP);
        }
        if(mHeartDataList.size() > 0 && heart_s)
        {
            foreach (QByteArray data, mHeartDataList)
            {
                heart_s->writeData(data);
            }
            mHeartDataList.clear();
        }

        msleep(200);
    }

    if(heart_s) {delete heart_s; heart_s = 0;}
    if(video_s) {delete video_s; video_s = 0;}
    if(report_s) {delete report_s; report_s = 0; }

}

ZCHXRadarDataServer::ZCHXRadarDataServer(zchxRadarOutputDataMgr* mgr, const zchxCommon::zchxRadarChannel& channel,
                                         const zchxVideoParserSettings& parse,
                                         const QString& local_ip,
                                         QObject *parent)
    : QObject(parent),
      mOutputMgr(mgr),
      mHeartTimer(0),
      mAutoOpenTimer(0),
      mVideoSocket(0),
      mReportSocket(0),
      mHeartSocket(0),
      mType(zchxCommon::RADAR_UNKNOWN),
      mOutputData(false),
      mVideoParse(0),
      mLastRecvIP(local_ip),
      mWorkThread(0)

{
    qRegisterMetaType<zchxCommon::zchxRadarChannelReport>("const zchxCommon::zchxRadarChannelReport&");
    mLocalIPList = zchxCommonUtils::getAllIpv4List();
    mCurrentIPIndex = mLocalIPList.indexOf(mLastRecvIP);
    if(mCurrentIPIndex < 0) mCurrentIPIndex = 0;

    updateChannelSettings(channel, parse);

    //创建默认的雷达数据输出目录
    QDir dir(QCoreApplication::applicationDirPath());
    if(!dir.exists(OUTPUT_DATA_DIR))
    {
        bool sts = dir.mkdir(OUTPUT_DATA_DIR);
        qDebug()<<"create output dir:"<<OUTPUT_DATA_DIR<<sts;
    } else
    {
        qDebug()<<"output dir already exist:"<<OUTPUT_DATA_DIR;
    }

//    if(!parent)
//    {
//        mWorkThread = new QThread;
//        moveToThread(mWorkThread);
//        mWorkThread->start();
//    }
    QTimer::singleShot(1000, this, SLOT(slotOpenRadar()));
    mHeartTimer = new QTimer(this);
    mHeartTimer->setInterval(3*1000);
    connect(mHeartTimer, SIGNAL(timeout()), this, SLOT(slotHeartTimerJob()));
    mHeartTimer->start();

    mAutoOpenTimer = new QTimer(this);
    mAutoOpenTimer->setInterval(6 * 1000);
    connect(mAutoOpenTimer, SIGNAL(timeout()), this, SLOT(slotOpenRadar()));
    mAutoOpenTimer->start();
}

ZCHXRadarDataServer::~ZCHXRadarDataServer()
{
    if(mHeartTimer)
    {
        mHeartTimer->stop();
        delete mHeartTimer;
        mHeartTimer = NULL;
    }
    if(mAutoOpenTimer)
    {
        delete mAutoOpenTimer;
        mAutoOpenTimer  = NULL;
    }


    if(mVideoSocket)
    {
        delete mVideoSocket;
        mVideoSocket = NULL;
    }
    if(mHeartSocket)
    {
        delete mHeartSocket;
        mHeartSocket = 0;
    }
    if(mReportSocket)
    {
        delete mReportSocket;
        mReportSocket = 0;
    }
    if(mVideoParse)
    {
        delete mVideoParse;
    }
    qDebug()<<"stop server work thread start...";
    if(mWorkThread && mWorkThread->isRunning())
    {
        mWorkThread->quit();
        mWorkThread->deleteLater();
        qDebug()<<"stop server work thread end...";
    }


    qDebug()<<"~ZCHXRadarDataServer() finished";
}

void ZCHXRadarDataServer::updateChannelSettings(const zchxCommon::zchxRadarChannel &channel,
                                                const zchxVideoParserSettings& parse)
{
    mChSet = channel;
    mChannelReport.mChannelID = mChSet.id;
    mParseSet = parse;
    updateParseSetting(mParseSet);
    initDataSocket();
}

void ZCHXRadarDataServer::initDataSocket()
{
    if(mLocalIPList.size() == 0) return;
    if(mVideoSocket)
    {
        delete mVideoSocket;
        mVideoSocket = 0;
    }
    if(mReportSocket)
    {
        delete mReportSocket;
        mReportSocket = 0;
    }
    if(mHeartSocket)
    {
        delete mHeartSocket;
        mHeartSocket = 0;
    }
    if(mCurrentIPIndex >= mLocalIPList.size() || mCurrentIPIndex < 0) mCurrentIPIndex = 0;
    QString ip_str = mLocalIPList[mCurrentIPIndex];
    mCurrentIPIndex++;
    mCurrentIPIndex = mCurrentIPIndex % mLocalIPList.size();

    QNetworkInterface nif = findNIF(ip_str);
    mHeartSocket = new zchxMulticastDataScoket(ip_str, mChSet.heart.ip, mChSet.heart.port, nif);
    connect(mHeartSocket, SIGNAL(signalSendRecvData(QByteArray)), this, SLOT(slotRecvHeartData(QByteArray)));
    mVideoSocket = new zchxMulticastDataScoket(ip_str, mChSet.video.ip, mChSet.video.port, nif);
    connect(mVideoSocket, SIGNAL(signalSendRecvData(QByteArray)), this, SLOT(slotRecvVideoData(QByteArray)));
    mReportSocket = new zchxMulticastDataScoket(ip_str, mChSet.report.ip, mChSet.report.port, nif);
    connect(mReportSocket, &zchxMulticastDataScoket::signalNoDataRecv, this, [=](){
        mVideoSocket->setIsOver(true);
        mHeartSocket->setIsOver(true);
        initDataSocket();
    }/*, Qt::DirectConnection*/);
    connect(mReportSocket, SIGNAL(signalSendRecvData(QByteArray)), this, SLOT(slotRecvReportData(QByteArray)));

    connect(mVideoSocket, &zchxMulticastDataScoket::signalNoDataRecv, this, [=](){
        mReportSocket->setIsOver(true);
        mHeartSocket->setIsOver(true);
        initDataSocket();
    }/*, Qt::DirectConnection*/);

}

void ZCHXRadarDataServer::updateParseSetting(const zchxVideoParserSettings &parse)
{
    mChannelReport.mRadarID = parse.radar_id;
    if(mVideoParse && !mVideoParse->isSameParseSetting(parse))
    {
#if 1
        mVideoParse->updateParseSetting(parse);
#else
        disconnect(this, SIGNAL(signalSendVideoData(QByteArray)), mVideoParse, SLOT(slotRecvVideoData(QByteArray)));
        delete mVideoParse;
        mVideoParse = 0;
#endif
    }
    if(!mVideoParse)
    {
        mVideoParse = new zchxRadarVideoParser(mOutputMgr, parse);
        connect(this, SIGNAL(signalSendVideoData(QByteArray)), mVideoParse, SLOT(slotRecvVideoData(QByteArray)));
    }
}

void ZCHXRadarDataServer::slotOpenRadar()//打开雷达
{
    if(!mHeartSocket) return;
    if(mVideoSocket && mVideoSocket->isFine() && mReportSocket && mReportSocket->isFine()) return;
    QByteArray line;
    line.resize(3);
    line[0] = 0x00;
    line[1] = 0xc1;
    line[2] = 0x01;
    mHeartSocket->writeData(line);
    line[0] = 0x01;
    line[1] = 0xc1;
    line[2] = 0x01;
    mHeartSocket->writeData(line);
}

void ZCHXRadarDataServer::slotCloseRadar()//关闭雷达
{
    if(!mHeartSocket) return;
    unsigned char Down[6] = { 0x00, 0xc1, 0x01, 0x01, 0xc1, 0x00 }; //00 c1 01  /01 c1 00
    mHeartSocket->writeData(QByteArray((char *)(Down + 0),3));
    mHeartSocket->writeData(QByteArray((char *)(Down + 3),3));
}

void ZCHXRadarDataServer::slotHeartTimerJob()
{
    if(!mHeartSocket)
    {
        qDebug()<<"write heart data. but socket null";
        return;
    }
    //修改发送数据方式
    QByteArray line;
    line.resize(2);
    line[0] = 0Xa0;
    line[1] = 0xc1;
    mHeartSocket->writeData(line);
    line[0] = 0x03;
    line[1] = 0xc2;
    mHeartSocket->writeData(line);
    line[0] = 0x04;
    line[1] = 0xc2;
    mHeartSocket->writeData(line);
    line[0] = 0x05;
    line[1] = 0xc2;
    mHeartSocket->writeData(line);
}

void ZCHXRadarDataServer::slotRecvHeartData(const QByteArray &bytes)
{
//    qDebug()<<"recv heart data with size:"<<bytes<<bytes.toHex().toUpper();
    if(mOutputData) outputData2File(Data_Heart, bytes);
}


void ZCHXRadarDataServer::outputData2File(int type, const QByteArray &data)
{
    QDir dir(QCoreApplication::applicationDirPath());
    dir.cd(OUTPUT_DATA_DIR);  //进入某文件夹
    QString *wkFileName = 0;
    QStringList *wkFileList;
    if(type == Data_Video)
    {
        if(!dir.exists(VIDEO_FILE_DIR))
        {
            dir.mkdir(VIDEO_FILE_DIR);
        }
        dir.cd(VIDEO_FILE_DIR);
        wkFileName = &mCurVideoFileName;
        wkFileList = &mVideoFileList;
    } else if(type == Data_Report)
    {
        if(!dir.exists(REPORT_FILE_DIR))
        {
            dir.mkdir(REPORT_FILE_DIR);
        }
        dir.cd(REPORT_FILE_DIR);
        wkFileName = &mCurReportFileName;
        wkFileList = &mReportFileList;
    } else if(type == Data_Heart)
    {
        if(!dir.exists(HEART_FILE_DIR))
        {
            dir.mkdir(HEART_FILE_DIR);
        }
        dir.cd(HEART_FILE_DIR);
        wkFileName = &mCurHeartFileName;
        wkFileList = &mHeartFileList;
    }
    if(!wkFileName || !wkFileList) return;
    if(wkFileName->isEmpty())
    {
        *wkFileName = QString("%1/%2.dat").arg(dir.absolutePath()).arg(QDateTime::currentMSecsSinceEpoch());
    }
    QFile file(*wkFileName);//创建文件对象
    bool isOk = file.open(QIODevice::WriteOnly |QIODevice::Append);
    if(false == isOk)
    {
        qDebug() <<"open file failed:"<<(*wkFileName);
        return;
    }
    file.write(data);

    qint64 size = file.size()/1024;
    if(size > FILE_MAX_SIZE) //当文本大于30M时 新建另一个文本写入数据
    {
        wkFileList->append(*wkFileName);
        if(wkFileList->size() >= WINRAR_FILE_SIZE)
        {
            compressFiles(*wkFileList, *wkFileName);
        }
        wkFileName->clear();
    }

    file.close();
}

void ZCHXRadarDataServer::slotRecvVideoData(const QByteArray &sRadarData)
{
    if(mOutputData)  outputData2File(Data_Video, sRadarData);
    if(mType == zchxCommon::RADAR_UNKNOWN) return;
    //发送数据到解析器进行解析
    emit signalSendVideoData(sRadarData);
}

void ZCHXRadarDataServer::setNormalCtrlValue(int infotype, int value)
{
    qDebug()<<"infotype:"<<infotype<<" value:"<<value;
    //检查当前值是否存在
    if(!mHeartSocket) return;
    switch (infotype) {
    case zchxCommon::POWER:
    {
        if(value == 0)
        {
            slotCloseRadar();
        } else if(value == 1)
        {
            slotOpenRadar();
        }
        break;
    }
    case zchxCommon::SCAN_SPEED:
    {
        UINT8 cmd[] = {0x0f, 0xc1, (UINT8)value  }; //00 c1 01  /01 c1 00
        mHeartSocket->writeData(QByteArray((char *)(cmd),sizeof(cmd)));
        break;
    }
    case zchxCommon::ANTENNA_HEIGHT:
    {
        int v = value * 1000;  // radar wants millimeters, not meters
        int v1 = v / 256;
        int v2 = v & 255;
        UINT8 cmd[10] = { 0x30, 0xc1, 0x01, 0, 0, 0, (UINT8)v2, (UINT8)v1, 0, 0 };
        mHeartSocket->writeData(QByteArray((char *)(cmd),sizeof(cmd)));
        break;
    }
    case zchxCommon::BEARING_ALIGNMENT:
    {
        if (value < 0)  value += 360;
        int v = value * 10;
        int v1 = v / 256;
        int v2 = v & 255;
        UINT8 cmd[4] = { 0x05, 0xc1, (UINT8)v2, (UINT8)v1 };
        mHeartSocket->writeData(QByteArray((char *)(cmd),sizeof(cmd)));
        break;
    }
    case zchxCommon::RANG:
    {
        unsigned int decimeters = (unsigned int)value * 10;
        UINT8 cmd[] = { 0x03,0xc1,
                        (UINT8)((decimeters >> 0) & 0XFFL),
                        (UINT8)((decimeters >> 8) & 0XFFL),
                        (UINT8)((decimeters >> 16) & 0XFFL),
                        (UINT8)((decimeters >> 24) & 0XFFL),
                      };
        mHeartSocket->writeData(QByteArray((char *)(cmd),sizeof(cmd)));
        break;
    }
    case zchxCommon::GAIN:
    {
        if(value < 0)
        {
            // 自动增益
            UINT8 cmd[] = {0x06, 0xc1, 0, 0, 0, 0, 0x01, 0, 0, 0, 0xad };
            mHeartSocket->writeData(QByteArray((char *)(cmd),sizeof(cmd)));
        } else if (value >= 0) {
            // Manual Gain
            int v = (value + 1) * 255 / 100;
            if (v > 255) v = 255;
            UINT8 cmd[] = { 0x06, 0xc1, 0, 0, 0, 0, 0, 0, 0, 0, (UINT8)v };
            mHeartSocket->writeData(QByteArray((char *)(cmd),sizeof(cmd)));
        }

        break;
    }
    case zchxCommon::SEA_CLUTTER:
    {
        if(value < 0)
        {
            // 自动
            UINT8 cmd[11] = { 0x06, 0xc1, 0x02, 0, 0, 0, 0x01, 0, 0, 0, 0xd3 };
            mHeartSocket->writeData(QByteArray((char *)(cmd),sizeof(cmd)));
        } else if (value >= 0) {
            // Manual
            int v = (value + 1) * 255 / 100;
            if (v > 255) v = 255;
            UINT8 cmd[] = { 0x06, 0xc1, 0x02, 0, 0, 0, 0, 0, 0, 0, (UINT8)v };
            mHeartSocket->writeData(QByteArray((char *)(cmd),sizeof(cmd)));
        }

        break;
    }

    case zchxCommon::RAIN_CLUTTER: // 8
    {
        int v = (value + 1) * 255 / 100;
        if (v > 255) v = 255;
        UINT8 cmd[] = { 0x06, 0xc1, 0x04, 0, 0, 0, 0, 0, 0, 0, (UINT8)v };
        mHeartSocket->writeData(QByteArray((char *)(cmd),sizeof(cmd)));
        break;
    }
    case zchxCommon::NOISE_REJECTION: // 9
    {
        UINT8 cmd[] = { 0x21, 0xc1, (UINT8)value };
        mHeartSocket->writeData(QByteArray((char *)(cmd),sizeof(cmd)));
        break;
    }
    case zchxCommon::SIDE_LOBE_SUPPRESSION: // 10
    {
        if (value < 0) {
            //自动
            UINT8 cmd[] = {0x06, 0xc1, 0x05, 0, 0, 0, 0x01, 0, 0, 0, 0xc0 };
            mHeartSocket->writeData(QByteArray((char *)(cmd),sizeof(cmd)));
        }else {
            int v = (value + 1) * 255 / 100;
            if (v > 255) v = 255;
            UINT8 cmd[] = { 0x6, 0xc1, 0x05, 0, 0, 0, 0, 0, 0, 0, (UINT8)v };
            mHeartSocket->writeData(QByteArray((char *)(cmd),sizeof(cmd)));
        }
        break;
    }
    case zchxCommon::INTERFERENCE_REJECTION: // 11
    {
        UINT8 cmd[] = { 0x08, 0xc1, (UINT8)value };
        mHeartSocket->writeData(QByteArray((char *)(cmd),sizeof(cmd)));
        break;
    }
    case zchxCommon::LOCAL_INTERFERENCE_REJECTION:  // 12
    {
        if (value < 0) value = 0;
        if (value > 3) value = 3;
        UINT8 cmd[] = { 0x0e, 0xc1, (UINT8)value };
        mHeartSocket->writeData(QByteArray((char *)(cmd),sizeof(cmd)));
        break;
    }
    case zchxCommon::TARGET_EXPANSION: // 13
    {
        UINT8 cmd[] = { 0x09, 0xc1, (UINT8)value };
        if(mType == zchxCommon::RADAR_6G)
        {
            cmd[0] = 0x12;
        }
        mHeartSocket->writeData(QByteArray((char *)(cmd),sizeof(cmd)));
        break;
    }
    case zchxCommon::TARGET_BOOST: // 14
    {
        UINT8 cmd[] = { 0x0a, 0xc1, (UINT8)value };
        mHeartSocket->writeData(QByteArray((char *)(cmd),sizeof(cmd)));
        break;
    }
    case zchxCommon::TARGET_SEPARATION: // 15
    {
        UINT8 cmd[] = { 0x22, 0xc1, (UINT8)value };
        mHeartSocket->writeData(QByteArray((char *)(cmd),sizeof(cmd)));
        break;
    }

    case zchxCommon::RESET:
    {
        qDebug()<<"reset radar now............";
        UINT8 cmd[] = {0x04, 0xc3};
        mHeartSocket->writeData(QByteArray((char *)(cmd),sizeof(cmd)));
    break;
    }
    default:
        break;
    }
}

void ZCHXRadarDataServer::setCtrlValue(int type, QJsonValue value)
{
    if(type < zchxCommon::FAN_IGNORE)
    {
        setNormalCtrlValue(type, value.toInt());
        return;
    }
    zchxCommon::zchxFanSector fan;
    QJsonArray array = value.toArray();
    if(array.size() < 2) return;
    fan.is_open = array[1].toBool();
    fan.sector = array[0].toInt();
    qDebug()<<array<<fan.is_open<<fan.sector;
    if(array.size() == 2)
    {
        UINT8 cmd[] = {0x0d, 0xc1, UINT8(fan.sector), 0, 0, 0, fan.is_open ? 0x01:0x00};
        mHeartSocket->writeData(QByteArray((char *)(cmd),sizeof(cmd)));
        qDebug()<<QByteArray((char *)(cmd),sizeof(cmd)).toHex().toUpper();
    } else if(array.size() == 4 && fan.is_open)
    {
        fan.start = array[2].toInt();
        fan.end = array[3].toInt();
        UINT8 cmd[] = {0x0d, 0xc1, UINT8(fan.sector), 0, 0, 0, 0x01,
                       (fan.start >> 8) & 0xFFL, (fan.start & 0xFFL),
                       (fan.end >> 8) & 0xFFL, (fan.end & 0xFFL),};
        mHeartSocket->writeData(QByteArray((char *)(cmd),sizeof(cmd)));
        qDebug()<<QByteArray((char *)(cmd),sizeof(cmd)).toHex().toUpper();

    }
}



void ZCHXRadarDataServer::updateReportValue(int controlType, int value)
{
    //检查值的范围
    if(controlType <= zchxCommon::UNKNOWN ||  controlType > zchxCommon::RESVERED)
    {
        return;
    }
    mChannelReport.time = QDateTime::currentDateTime().toString("yyyyMMdd hh:mm:ss");
    int index = -1;
    for(int i=0; i<mChannelReport.reportList.size(); i++)
    {
        if(mChannelReport.reportList[i].type == controlType)
        {
            index = i;
            break;
        }
    }
    zchxCommon::zchxRadarReportData data;
    data.timeOfDay = QDateTime::fromString(mChannelReport.time, "yyyyMMdd hh:mm:ss").toMSecsSinceEpoch();
    data.type = controlType;
    data.jsval = QJsonValue(value);
    if(index == -1) mChannelReport.reportList.append(data);
    else mChannelReport.reportList.replace(index, data);
    return;
}

void ZCHXRadarDataServer::slotRecvReportData(const QByteArray& bytes)
{
//    qDebug()<<"recv report data:"<<bytes.size();
    if(mOutputData) outputData2File(Data_Report, bytes);
    int len = bytes.size();
    if(len < 3 ) return;
    unsigned char val = bytes[1];
//    qDebug()<<"recv report data with head:"<<bytes.mid(1,1).toHex().toUpper()<<" and total:"<<bytes.toUpper();
    if(val != 0xC4) return;

    switch ((len << 8) + bytes[0])
    {
    case (18 << 8) + 0x01:  //4068 + 1
    {
        RadarReport_01C4_18 *s = (RadarReport_01C4_18 *)bytes.data();
//        qDebug()<<"radar status:"<<s->radar_status;
        switch (bytes[2])
        {
        case 0x01:
//            qDebug()<<"reports status RADAR_STANDBY";
            updateReportValue(zchxCommon::POWER,0);
            break;
        case 0x02:
//            qDebug()<<"reports status TRANSMIT";
            updateReportValue(zchxCommon::POWER,1);
            break;
        case 0x05:
//            qDebug()<<"reports status WAKING UP";
            break;
        default:
            break;
        }
        break;
    }
    case (99 << 8) + 0x02: // length 99, 02 C4,contains gain,rain,interference rejection,sea
        //target_boost, target_expansion,range
    {
        //cout<<"进来了_2 02，C4，包含增益，雨，干扰抑制，海洋,target_boost, target_expansion,range";
        RadarReport_02C4_99 *s = (RadarReport_02C4_99 *)bytes.data();
        //gain
        if (s->field8 == 1)        // 1 for auto
            updateReportValue(zchxCommon::GAIN,-1);
        else
            updateReportValue(zchxCommon::GAIN, s->gain * 100 / 255);
        //sea
        if (s->field13 == 0x01)
            updateReportValue(zchxCommon::SEA_CLUTTER,-1);  // auto sea
        else
            updateReportValue(zchxCommon::SEA_CLUTTER,s->sea * 100 / 255);
        //rain
        updateReportValue(zchxCommon::RAIN_CLUTTER, s->rain * 100 / 255);
        //target boost
        updateReportValue(zchxCommon::TARGET_BOOST, s->target_boost);
        //s->interference rejection
        updateReportValue(zchxCommon::INTERFERENCE_REJECTION, s->interference_rejection);
        //target expansion
        updateReportValue(zchxCommon::TARGET_EXPANSION, s->target_expansion);
        //range
        updateReportValue(zchxCommon::RANG, s->range / 10);
        break;
    }
    case (129 << 8) + 0x03: // 129 bytes starting with 03 C4
    {
        //cout<<"进来了_3 129 bytes starting with 03 C4";
        RadarReport_03C4_129 *s = (RadarReport_03C4_129 *)bytes.data();
        switch (s->radar_type) {
        case 0x0f:
//            qDebug()<<"current radar is BR24";
            mType = zchxCommon::RADAR_BR24;
            break;
        case 0x08:
//            qDebug()<<"current radar is 3G";
            mType = zchxCommon::RADAR_3G;
            break;
        case 0x01:
//            qDebug()<<"current radar is 4G";
            mType = zchxCommon::RADAR_4G;
            break;
        case 0x00:
//            qDebug()<<"current radar is 6G";
            mType = zchxCommon::RADAR_6G;
            break;
        default:
            qDebug()<<"Unknown radar_type: "<< s->radar_type;
        }
        if(mOutputMgr) mOutputMgr->updateRadarType(mParseSet.channel_id, mType, QString::number(mParseSet.radar_id));
        if(mVideoParse) mVideoParse->slotSetRadarType(mType);
        break;
    }
    case (66 << 8) + 0x04: // 66 bytes starting with 04 C4,contains bearing alignment,antenna height
    {
        //cout<<"进来了_4 从04 C4开始的66个字节，包含轴承对齐，天线高度";
        RadarReport_04C4_66 *data = (RadarReport_04C4_66 *)bytes.data();
        // bearing alignment
        int ba = (int)data->bearing_alignment / 10;
        if (ba > 180) ba = ba - 360;
        updateReportValue(zchxCommon::BEARING_ALIGNMENT, ba);
        // antenna height
        updateReportValue(zchxCommon::ANTENNA_HEIGHT, data->antenna_height / 1000);
        break;
    }
    case (564 << 8) + 0x05:
    {
        //cout<<"进来了_5 内容未知，但我们知道BR24雷达发送这个";
        // Content unknown, but we know that BR24 radomes send this
        //ZCHXLOG_DEBUG("Navico BR24: msg");
        break;
    }
    case (18 << 8) + 0x08: // length 18, 08 C4,
        //contains scan speed, noise rejection and target_separation and sidelobe suppression,local_interference_rejection
    {
        //cout<<"进来了_6 包含扫描速度，噪声抑制和目标分离和侧面抑制，局部干扰抑制";
        RadarReport_08C4_18 *s08 = (RadarReport_08C4_18 *)bytes.data();
        updateReportValue(zchxCommon::SCAN_SPEED, s08->scan_speed);
        updateReportValue(zchxCommon::NOISE_REJECTION, s08->noise_rejection);
        updateReportValue(zchxCommon::TARGET_SEPARATION, s08->target_sep);
        if (s08->sls_auto == 1)
            updateReportValue(zchxCommon::SIDE_LOBE_SUPPRESSION,-1);
        else
            updateReportValue(zchxCommon::SIDE_LOBE_SUPPRESSION, s08->side_lobe_suppression * 100 / 255);
        updateReportValue(zchxCommon::LOCAL_INTERFERENCE_REJECTION, s08->local_interference_rejection);
        break;
    }
    case (21 << 8) + 0x08:
    {  // length 21, 08 C4
        // contains Doppler data in extra 3 bytes
        RadarReport_08C4_21 *s08 = (RadarReport_08C4_21 *)bytes.data();
        updateReportValue(zchxCommon::SCAN_SPEED, s08->old.scan_speed);
        updateReportValue(zchxCommon::NOISE_REJECTION, s08->old.noise_rejection);
        updateReportValue(zchxCommon::TARGET_SEPARATION, s08->old.target_sep);
        if (s08->old.sls_auto == 1)
            updateReportValue(zchxCommon::SIDE_LOBE_SUPPRESSION,-1);
        else
            updateReportValue(zchxCommon::SIDE_LOBE_SUPPRESSION, s08->old.side_lobe_suppression * 100 / 255);
        updateReportValue(zchxCommon::LOCAL_INTERFERENCE_REJECTION, s08->old.local_interference_rejection);
        uint8_t state = s08->doppler_state;
        uint8_t speed = s08->doppler_speed;
        qDebug()<<"doppler state:"<<state<<speed;
        break;
    }
    default:
        break;
    }

    //更新雷达状态
    if(mOutputMgr) mOutputMgr->updateChannelReport(mParseSet.channel_id, mChannelReport);

    return;
}


void ZCHXRadarDataServer::parseRadarControlSetting(int infotype)
{
//    QString str_radar_cmd = QString("Radar_Command_%1").arg(m_uSourceID);
//    QStringList list = Utils::Profiles::instance()->value(str_radar_cmd,\
//                                                           RadarStatus::getTypeString(infotype,STR_MODE_ENG)\
//                                                          ).toStringList();
//    if(list.length() < 2) return;
//    mReportStatusMap[infotype] = RadarStatus(infotype, list[0].toInt(), list[1].toInt());
}

void ZCHXRadarDataServer::compressFiles(QStringList &list, QString &fileName)
{
    if(fileName.size() > 0 && !list.contains(fileName))
    {
        list.append(fileName);
    }
    if(list.size() == 0) return;
    zchxRadarVideoWinRarThread* thread = new zchxRadarVideoWinRarThread(list);
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    list.clear();
    thread->start();
    fileName.clear();
}

void ZCHXRadarDataServer::slotPrintRecvData(bool sts)//打印回波数据
{
    mOutputData = sts;
    if(!mOutputData)
    {
        compressFiles(mVideoFileList, mCurVideoFileName);
        compressFiles(mReportFileList, mCurReportFileName);
        compressFiles(mHeartFileList, mCurHeartFileName);
    }
}

void ZCHXRadarDataServer::updateFilterAreaSettings(bool sts, const zchxCommon::zchxfilterAreaList &list)
{
    mParseSet.filter_enable = sts;
    mParseSet.filter_area = list;
    updateParseSetting(mParseSet);
}

