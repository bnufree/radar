#ifndef ZCHXDATAOUTPUTSERVERMGR_H
#define ZCHXDATAOUTPUTSERVERMGR_H

#include <QObject>
#include <QMap>
#include "zchxdatadef.h"
#include "zchxdataoutputserverthread.h"

class QTimer;

#define     PUBLISH_INS       zchxDataOutputServerMgr::instance()

class zchxDataOutputServerMgr : public QObject
{
    Q_OBJECT
private:
    explicit zchxDataOutputServerMgr(QObject *parent = 0);
public:
    ~zchxDataOutputServerMgr();
    void*    getCtx() {return mCtx;}
    static zchxDataOutputServerMgr* instance();
private slots:
    void   appendAisData(const QByteArray& data);
    void   appendVideoData(const QByteArray& data);
    void   appendTrackData(const QByteArray& data);
    void   appendFilterData(const QByteArray& data);
    void   appendReportData(const QByteArray& data) {slotAppendData(zchxCommon::THREAD_DATA_RADAR_REPORT, data);}
    void   appendCfgData(const QByteArray& data) {slotAppendData(zchxCommon::THREAD_DATA_SVRCFG, data);}
public slots:
    void   initFromCfg(const zchxCommon::zchxPublishSettingsList& list);
    void   slotAppendData(int caseid, const QByteArray& data);


private:
    zchxDataOutputServerThread* getThread(int port, const QString& topic);


signals:    
    void signalSendPortStartStatus(const zchxCommon::zchxPortStatusList& list);
    void signalInitPublish(const zchxCommon::zchxPublishSettingsList& list);
    void signalSendPublishData(int type, const QByteArray& data);

private:
    static zchxDataOutputServerMgr* m_pInstace;
    QMap<int, zchxDataOutputServerThread*>          mPortThreadList;        //通过端口管理的所有线程
    QMap<int, zchxDataOutputServerThread*>          mCaseThreadList;        //每一个pub对象的输出线程
    QMap<int, QString>                              mCaseTopicList;         //每一个pub对象的topic
    void*                                           mCtx;
};

#endif // ZCHXDATAOUTPUTSERVERMGR_H
