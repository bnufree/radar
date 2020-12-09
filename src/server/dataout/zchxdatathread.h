#ifndef ZCHXDATATHREAD_H
#define ZCHXDATATHREAD_H

#include <QThread>
#include "zmq.h"

//struct PortStatus{
//    int     port;
//    bool    sts;
//    QString topic;
//};
//typedef QList<PortStatus>       PortStatusList;

//Q_DECLARE_METATYPE(PortStatus)
//Q_DECLARE_METATYPE(PortStatusList)

class ZmqMonitorThread;

class zchxDataThread : public QThread
{
    Q_OBJECT
public:
    explicit zchxDataThread(int type, void* ctx, int port, bool monitor = false, QObject *parent = 0);
    ~zchxDataThread();

    bool     isOK() const {return mIsOk;}
    int      getPort() const {return mPort;}

    void     stopMe() {mStop = true;}
private:
    virtual bool     init();
signals:
protected:
    int                     mPort;
    void                    *mCtx;
    bool                    mIsOk;
    bool                    mStop;
    void                    *mSocket;
    QString                 mUrl;
    ZmqMonitorThread        *mMonitorThread;
    bool                    mMonitorClient;
    int                     mZmqType;
};

#endif // ZCHXDATATHREAD_H
