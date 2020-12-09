#ifndef ZCHXREQUESTWORKER_H
#define ZCHXREQUESTWORKER_H

#include <QObject>
#include <QThread>

class QTimer;
class zchxRequestWorker : public QObject
{
    Q_OBJECT
public:
    explicit zchxRequestWorker(QObject *parent = 0);
    ~zchxRequestWorker();
    bool     isOk() const {return mIsLogin;}

signals:
    void     signalSetHostInfo(const QString& ip, int port);
    void     signalSendErrorMsg(const QString& msg, int timeout = 0);
    void     signalRequestCfgInfo();
    void     signalReConnect();    
    void     signalRecheckHostPort(const QString& host, int port);
    void     signalSendSvrCfg(const QJsonObject& obj);
    void     signalUpdateSvrCfg(const QJsonValue& val);
    void     signalUpdateCfg(int cmd, const QJsonValue& val);
    void     signalSetRadarCtrl(const QJsonValue& val);

public slots:
    void     slotSetHostInfo(const QString& ip, int port);
    void     slotRequestCfgInfo();
    void     slotHeartMsg();
    void     slotUpdateSvrCfg(const QJsonValue& val);
    void     slotUpdateCfg(int cmd, const QJsonValue& val);
    void     slotSetRadarCtrl(const QJsonValue& val);
private:
    QString         now();
    bool            sendMsgAndWaitReply(QJsonValue* ret, int cmd, const QJsonValue& obj);

private:
    QThread     mWorkThread;
    void*       mSocket;
    void*       mCtx;
    bool        mIsLogin;
    QString     mHost;
    int         mPort;
    QTimer*     mHeartTimer;
};

#endif // ZCHXREQUESTWORKER_H
