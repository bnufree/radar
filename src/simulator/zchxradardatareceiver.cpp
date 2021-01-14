#include "zchxradardatareceiver.h"
#include "zchxmulticastdatasocket.h"
#include "zchxradarvideowinrarthread.h"
#include <QDir>
#include <QCoreApplication>

#ifdef Q_OS_UNIX
    typedef uint8_t     UINT8;
#endif

#define WINRAR_FILE_SIZE 20
#define VIDEO_FILE_DIR      "VideData"
#define REPORT_FILE_DIR     "ReportData"
#define HEART_FILE_DIR      "HeartData"
#define OUTPUT_DATA_DIR     "OutputData"
#define FILE_MAX_SIZE       30720
typedef uint8_t UINT8;

//#pragma comment(lib, "ws2_32.lib")

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

QStringList getAllIpv4List()
{
    QStringList list;
    //获取当前本机的ipv4的ip;
    //获取所有网络接口的列表
    foreach (QNetworkInterface netInterface, QNetworkInterface::allInterfaces())
    {
        QList<QNetworkAddressEntry> entryList = netInterface.addressEntries();
        foreach(QNetworkAddressEntry entry, entryList)
        {
            QHostAddress ip = entry.ip();
            if(ip.protocol() != QAbstractSocket::IPv4Protocol) continue;
            QString ip_str = ip.toString();
            if(ip_str.startsWith("127.")) continue;
//            if(ip_str.startsWith("169.254")) continue;
            list.append(ip_str);
        }
    }

    return list;
}

zchxRadarDataFinder::zchxRadarDataFinder(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<QList<zchxRadarDev>>("const QList<zchxRadarDev>&");
    mLocalIPList = getAllIpv4List();
    mCurIPIndex = 0;
    mCommonIP = "236.6.7.5";
    mCommonPort = 6878;
    mWorkThread = 0;
    mSocket = 0;

    init();

    if(mWorkThread) this->moveToThread(mWorkThread);
}

void zchxRadarDataFinder::init()
{
    if(mCurIPIndex >= mLocalIPList.size()) return;
    if(mSocket) delete mSocket;
    QString ip_str = mLocalIPList[mCurIPIndex];
    QNetworkInterface nif = findNIF(ip_str);
    mLocalIP = ip_str;

    mSocket = new zchxMulticastDataScoket(ip_str, mCommonIP, mCommonPort, true, nif);
    connect(mSocket, SIGNAL(signalSendRecvData(QByteArray)), this, SLOT(slotRecvData(QByteArray)));
    connect(mSocket, &zchxMulticastDataScoket::signalNoDataRecv, this, [=](){
        mCurIPIndex++;
        if(mCurIPIndex == mLocalIPList.size())
        {
            qDebug()<<"all local ip has been searched. but no data recved. so use default dev(236.6.7.8:6678) now";
            QList<zchxRadarDev> list;
            zchxRadarDev infoA;
            infoA.id = "default_236_6_7_8";
            infoA.mVideo.addr = "236.6.7.8";
            infoA.mVideo.port = 6678;
            infoA.mReport.addr = "236.6.7.9";
            infoA.mReport.port = 6679;
            infoA.mHeart.addr = "236.6.7.10";
            infoA.mHeart.port = 6680;
            list.append(infoA);

            if(list.size() > 0)
            {
                emit signalSendResult(list, "");
            }

        } else
        {
            init();
        }
    });
}

bool PackedAddress2Normal(QString& ip, int &port, const PackedAddress& src)
{
    uint8_t *a = (uint8_t *)&(src.addr);  // sin_addr is in network layout
    ip = QString("").sprintf("%u.%u.%u.%u", a[0], a[1], a[2], a[3]);
    port = ntohs(src.port);

    return true;
}

void zchxRadarDataFinder::slotRecvData(const QByteArray& bytes)
{
    QList<zchxRadarDev> list;
    if (bytes[0] == 0x01 && bytes[1] == 0xB1)
    {
        qDebug()<<"radar wake now.......";
        return;
    }
    if (bytes[0] == 0x01 && bytes[1] == 0xB2) {  // Common Navico message from 4G++
        qDebug()<<"radar data address info return now";
        RadarReport_01B2 *data = (RadarReport_01B2 *)bytes.data();

        zchxRadarDev infoA;
        infoA.id = QString::fromStdString(data->serialno) + "_A";
        PackedAddress2Normal(infoA.mVideo.addr, infoA.mVideo.port, data->addrDataA);
        PackedAddress2Normal(infoA.mHeart.addr, infoA.mHeart.port, data->addrSendA);
        PackedAddress2Normal(infoA.mReport.addr, infoA.mReport.port, data->addrReportA);
        list.append(infoA);
        if(bytes.size() > 150)
        {
            zchxRadarDev infoB;
            infoB.id = QString::fromStdString(data->serialno)+ "_B";
            PackedAddress2Normal(infoB.mVideo.addr, infoB.mVideo.port, data->addrDataB);
            PackedAddress2Normal(infoB.mHeart.addr, infoB.mHeart.port, data->addrSendB);
            PackedAddress2Normal(infoB.mReport.addr, infoB.mReport.port, data->addrReportB);
            list.append(infoB);
        }
    }

    qDebug()<<"recv device list size:"<<list.size()<<" recv ip:"<<mLocalIP;
    for(int i=0; i<list.size();i++)
    {
        zchxRadarDev dev = list[i];
        qDebug()<<"id:"<<dev.id<<" video:"<<dev.mVideo.addr<<dev.mVideo.port<<" report:"<<dev.mReport.addr<<dev.mReport.port<<" heart:"<<dev.mHeart.addr<<dev.mHeart.port;
    }

    if(list.size() > 0)
    {
        emit signalSendResult(list, mLocalIP);
    }
}

ZCHXRadarDataReceiver::ZCHXRadarDataReceiver(const zchxRadarDev& dev,
                                         const QString& local_ip,
                                         QObject *parent)
    : QObject(parent),
      mHeartTimer(0),
      mAutoOpenTimer(0),
      mVideoSocket(0),
      mReportSocket(0),
      mHeartSocket(0),
      mType(0),
      mOutputData(false),
      mLocalIP(local_ip),
      mWorkThread(0),
      mDev(dev)

{
    mLocalIPList = getAllIpv4List();
    mCurIPIndex = mLocalIP.indexOf(mLocalIP);
    if(mCurIPIndex < 0) mCurIPIndex = 0;
    init();

    //创建默认的雷达数据输出目录
    QDir dir(QCoreApplication::applicationDirPath());
    mDataSaveDir = QString("%1/%2/").arg(OUTPUT_DATA_DIR).arg(mDev.id);
    if(!dir.exists(mDataSaveDir))
    {
        bool sts = dir.mkpath(mDataSaveDir);
        qDebug()<<"create output dir:"<<mDataSaveDir<<sts;
    } else
    {
        qDebug()<<"output dir already exist:"<<mDataSaveDir;
    }
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

ZCHXRadarDataReceiver::~ZCHXRadarDataReceiver()
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
    qDebug()<<"stop server work thread start..."<<mWorkThread;
    if(mWorkThread && mWorkThread->isRunning())
    {
        mWorkThread->quit();
        mWorkThread->deleteLater();
        qDebug()<<"stop server work thread end...";
    }


    qDebug()<<"~ZCHXRadarDataReceiver() finished";
}

void ZCHXRadarDataReceiver::init()
{
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

    mLocalIP = mLocalIPList[mCurIPIndex++];
    QNetworkInterface nif = findNIF(mLocalIP);
    mHeartSocket = new zchxMulticastDataScoket(mLocalIP, mDev.mHeart.addr, mDev.mHeart.port, true, nif);
    connect(mHeartSocket, SIGNAL(signalSendRecvData(QByteArray)), this, SLOT(slotRecvHeartData(QByteArray)));
    mVideoSocket = new zchxMulticastDataScoket(mLocalIP, mDev.mVideo.addr, mDev.mVideo.port, true,  nif);
    connect(mVideoSocket, SIGNAL(signalSendRecvData(QByteArray)), this, SLOT(slotRecvVideoData(QByteArray)));
    mReportSocket = new zchxMulticastDataScoket(mLocalIP, mDev.mReport.addr, mDev.mReport.port, true, nif);
    connect(mReportSocket, &zchxMulticastDataScoket::signalNoDataRecv, this, [=](){
        qDebug()<<"no data recved from report socket";
        init();
    });
    connect(mReportSocket, SIGNAL(signalSendRecvData(QByteArray)), this, SLOT(slotRecvReportData(QByteArray)));

    connect(mVideoSocket, &zchxMulticastDataScoket::signalNoDataRecv, this, [=](){
        qDebug()<<"no data recved from video socket";
        init();
    });

}

void ZCHXRadarDataReceiver::slotOpenRadar()//打开雷达
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

void ZCHXRadarDataReceiver::slotCloseRadar()//关闭雷达
{
    if(!mHeartSocket) return;
    unsigned char Down[6] = { 0x00, 0xc1, 0x01, 0x01, 0xc1, 0x00 }; //00 c1 01  /01 c1 00
    mHeartSocket->writeData(QByteArray((char *)(Down + 0),3));
    mHeartSocket->writeData(QByteArray((char *)(Down + 3),3));
}

void ZCHXRadarDataReceiver::slotHeartTimerJob()
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

void ZCHXRadarDataReceiver::slotRecvHeartData(const QByteArray &bytes)
{
//    qDebug()<<"recv heart data with size:"<<bytes<<bytes.toHex().toUpper();
    if(mOutputData) outputData2File(Data_Heart, bytes);
}

//每一次写入的数据长度以及时间，便于模拟时真实的时间间隔和数据发送
void ZCHXRadarDataReceiver::outputData2File(int type, const QByteArray &data)
{
    QDir dir(mDataSaveDir);
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
    qint64 time = QDateTime::currentMSecsSinceEpoch();
    qint64 data_size = data.size();
    QByteArray contents;
    contents.resize(sizeof(qint64) * 2);
    memcpy(contents.data(), &time, sizeof(qint64));
    memcpy(contents.data() + sizeof(qint64), &data_size, sizeof(qint64));
    file.write(contents);
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

void ZCHXRadarDataReceiver::slotRecvVideoData(const QByteArray &sRadarData)
{
    if(mOutputData)  outputData2File(Data_Video, sRadarData);
}

void ZCHXRadarDataReceiver::slotRecvReportData(const QByteArray& bytes)
{
//    qDebug()<<"recv report data:"<<bytes.size();
    if(mOutputData) outputData2File(Data_Report, bytes);
    return;
}


void ZCHXRadarDataReceiver::compressFiles(QStringList &list, QString &fileName)
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

void ZCHXRadarDataReceiver::slotPrintRecvData(bool sts)//打印回波数据
{
    qDebug()<<"print data status:"<<sts;
    mOutputData = sts;
    if(!mOutputData)
    {
        qDebug()<<"start compress all files";
        compressFiles(mVideoFileList, mCurVideoFileName);
        compressFiles(mReportFileList, mCurReportFileName);
        compressFiles(mHeartFileList, mCurHeartFileName);
    }
}


