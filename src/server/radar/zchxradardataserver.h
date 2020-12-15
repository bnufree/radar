#ifndef ZCHXRADARDATASERVER_H
#define ZCHXRADARDATASERVER_H

#include <QObject>
#include <QTimer>
#include <QThread>
#include "zchxradarcommon.h"
#include "radarccontroldefines.h"
#include "zchxradaroutputdatamgr.h"
#include <QNetworkInterface>


class zchxMulticastDataScoket;
class zchxRadarVideoParser;
class QUdpSocket;


enum RadarDataType{
    Data_Video = 0,
    Data_Report,
    Data_Heart,
};


class zchxRadarDataController : public QThread
{
    Q_OBJECT
public:
    enum RADAR_DATA_TYPE{
        DATA_HEART = 0,
        DATA_REPORT,
        DATA_VIDEO
    };

    explicit zchxRadarDataController(const zchxCommon::zchxRadarChannel& channel,
                                 const QString& local_ip = QString(),
                                 QObject *parent = 0);
    ~zchxRadarDataController();
    void writeData(const QByteArray& bytes, int data_type = DATA_HEART);
    void updateNewSetting(const zchxCommon::zchxRadarChannel& channel);
    void setOver(bool sts) {mIsOver = sts;}
    void stop();
signals:
    void signalSendVideoData(const QByteArray& bytes);
    void signalSendReportData(const QByteArray& bytes);
    void signalSendHeartData(const QByteArray& bytes);
    void signalSendRecvIP(const QString& ip);
private:
    void run();

private:
    QString                             mVideoIP;
    int                                 mVideoPort;
    QString                             mReportIP;
    int                                 mReportPort;
    QString                             mHeartIP;
    int                                 mHeartPort;
    QString                             mLocalIP;
    QStringList                         mLocalIPList;
    QTimer*                             mDataChkTimer;
    int                                 mCurIPIndex;
    bool                                mHostChange;
    bool                                mIsOver;
    QByteArrayList                      mHeartDataList;
};

class ZCHXRadarDataServer : public QObject
{
    Q_OBJECT
public:
    explicit ZCHXRadarDataServer(zchxRadarOutputDataMgr* mgr, const zchxCommon::zchxRadarChannel& channel,
                                 const zchxVideoParserSettings& parse,
                                 const QString& local_ip = QString(),
                                 QObject *parent = 0);
    ~ZCHXRadarDataServer();

    void updateChannelSettings(const zchxCommon::zchxRadarChannel& channel, const zchxVideoParserSettings& parse);
    void updateFilterAreaSettings(bool sts, const zchxCommon::zchxfilterAreaList& list);

signals:
    void signalSendRecvedContent(qint64 time, const QString& name, const QString& content);
    void signalSendVideoData(const QByteArray& bytes);
    void signalSendRecvIP(const QString& ip);

public slots:
    void slotPrintRecvData(bool);//打印回波数据
    void slotOpenRadar();
    void slotCloseRadar();
    //4g雷达 IP 236.6.7.10  port 6680
    //6g雷达 IP 236.6.7.100  port 6132
    //开关控制 IP 236.6.101.100  port 6133
    void slotHeartTimerJob();//心跳通信
    //雷达控制
    void setNormalCtrlValue(int infotype, int value);
    void setCtrlValue(int type, QJsonValue value);
    void updateReportValue(int controlType, QJsonValue value);

private slots:
    void slotRecvVideoData(const QByteArray &sRadarData);//解析
    void slotRecvReportData(const QByteArray& bytes);
    void slotRecvHeartData(const QByteArray& bytes);
    void initDataSocket();
private:
    void outputData2File(int type, const QByteArray& data);
    void compressFiles(QStringList& list, QString& fileName);
    void parseRadarControlSetting(int infotype);
    void updateParseSetting(const zchxVideoParserSettings& set);

private:
    zchxCommon::zchxRadarChannel          mChSet;
    zchxVideoParserSettings             mParseSet;
    zchxRadarVideoParser                *mVideoParse;
    //发送心跳信息
//    bool                                mHearEnable;
    QTimer*                             mHeartTimer;
//    int                                 mHeartTimeInterval;
    //雷达断开后自动打开
//    bool                                mAutoOpenEnable;
//    int                                 mAutoOpenInerval;
    QTimer*                             mAutoOpenTimer;
    //雷达的基本信息
    int                                 mType;           //3G, 4G, 6G

    zchxCommon::zchxRadarChannelReport                 mChannelReport; //雷达状态容器
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
    zchxRadarOutputDataMgr*         mOutputMgr;
    QStringList                     mLocalIPList;
    int                             mCurrentIPIndex;
    QString                         mLastRecvIP;
    QString                         mDataSaveDir;
};

#endif // ZCHXRADARDATASERVER_H
