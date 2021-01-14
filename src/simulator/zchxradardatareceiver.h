#ifndef ZCHXRADARDATARECEIVER_H
#define ZCHXRADARDATARECEIVER_H

#include <QObject>
#include <QTimer>
#include <QThread>
#include "radarccontroldefines.h"
#include <QNetworkInterface>


class zchxMulticastDataScoket;

enum RadarDataType{
    Data_Video = 0,
    Data_Report,
    Data_Heart,
};

struct zchxSocketAddr{
    int type;
    QString addr;
    int port;
};

struct zchxRadarDev{
    QString id;
    zchxSocketAddr      mVideo;
    zchxSocketAddr      mReport;
    zchxSocketAddr      mHeart;
};

class zchxRadarDataFinder : public QObject
{
    Q_OBJECT
public:

    explicit zchxRadarDataFinder(QObject *parent = 0);
    ~zchxRadarDataFinder() {}

private:
    void init();
public slots:
    void slotRecvData(const QByteArray& bytes);
signals:
    void signalSendResult(const QList<zchxRadarDev>& list, const QString& ip);

private:
    QString     mCommonIP;
    int         mCommonPort;
    QStringList mLocalIPList;
    QString     mLocalIP;
    int         mCurIPIndex;
    QThread*    mWorkThread;
    zchxMulticastDataScoket*  mSocket;
};

class ZCHXRadarDataReceiver : public QObject
{
    Q_OBJECT
public:
    explicit ZCHXRadarDataReceiver(const zchxRadarDev& dev,
                                 const QString& local_ip,
                                 QObject *parent = 0);
    ~ZCHXRadarDataReceiver();


    bool isDataOutput() const {return mOutputData;}
signals:
    void signalSendRecvedContent(qint64 time, const QString& name, const QString& content);
    void signalSendVideoData(const QByteArray& bytes);
    void signalSendRecvIP(const QString& ip);
    void signalSendHeadChangedData(int head);
    void signalSendScanTime(double secs);

public slots:
    void slotPrintRecvData(bool);//打印回波数据
    void slotOpenRadar();
    void slotCloseRadar();
    //4g雷达 IP 236.6.7.10  port 6680
    //6g雷达 IP 236.6.7.100  port 6132
    //开关控制 IP 236.6.101.100  port 6133
    void slotHeartTimerJob();//心跳通信

private slots:
    void init();
    void slotRecvVideoData(const QByteArray &sRadarData);//解析
    void slotRecvReportData(const QByteArray& bytes);
    void slotRecvHeartData(const QByteArray& bytes);
private:
    void outputData2File(int type, const QByteArray& data);
    void compressFiles(QStringList& list, QString& fileName);

private:
    zchxRadarDev          mDev;
    QTimer*                             mHeartTimer;
    QTimer*                             mAutoOpenTimer;
    //雷达的基本信息
    int                                 mType;           //3G, 4G, 6G
    zchxMulticastDataScoket*            mVideoSocket;
    zchxMulticastDataScoket*            mReportSocket;
    zchxMulticastDataScoket*            mHeartSocket;
    QThread*                            mWorkThread;

    //
    bool                            mOutputData;
    QString                         mCurReportFileName;
    QString                         mCurVideoFileName;
    QString                         mCurHeartFileName;
    QStringList                     mVideoFileList;
    QStringList                     mReportFileList;
    QStringList                     mHeartFileList;
    QStringList                     mLocalIPList;
    QString                         mLocalIP;
    int                             mCurIPIndex;
    QString                         mDataSaveDir;
};

#endif // ZCHXRADARDATARECEIVER_H
