#ifndef ZCHXMULTICASTDATASOCKET_H
#define ZCHXMULTICASTDATASOCKET_H

#include <QObject>
#include <QUdpSocket>
#include <QTimer>
#include <QDateTime>
#include <QNetworkInterface>
#include <QThread>

enum DataRecvMode{
    ModeAsyncRecv = 0,  //默认是异步接收
    ModeSyncRecv,       //同步接收
};

#define         Video_Tag           "Video Data"
#define         Heart_Tag           "Heart Data"
#define         Report_Tag          "Report Data"

#undef         UDP_THREAD

#ifndef UDP_THREAD
class zchxMulticastDataScoket : public QUdpSocket
{
    Q_OBJECT
public:
    explicit zchxMulticastDataScoket(const QString & ip, const  QString& host,
                                     int    port,                                     
                                     bool data_recv_chk,
                                     const QNetworkInterface nif = QNetworkInterface(),
                                     QObject *parent = 0);

    virtual ~zchxMulticastDataScoket();

    bool isFine() const;
    bool isInit() const {return mInit;}
    QString getHost() const {return mHost;}
    int getPort() const {return mPort;}
    bool isSameHost(const QString host, int port) const
    {
        return host == getHost() && port == getPort();
    }
    void    setDebug(bool sts) {mDebug = sts;}
private slots:
    virtual void init();

signals:
    void signalSendRecvData(const QByteArray& data);
    void signalNoDataRecv();
public slots:
    virtual void slotReadyReadMulticastData();
    virtual void slotDisplayUdpReportError(QAbstractSocket::SocketError);
    virtual void slotStateChanged(QAbstractSocket::SocketState);
    void    writeData(const QByteArray& data);
    void    slotCheckDataRecv();

private:
    QString             mHost;
    int                 mPort;
    QString             mLocalIP;
    bool                mInit;
    bool                mIsOk;
    QDateTime           mLastRecvTime;
    QNetworkInterface   mNIF;
    int                 mErrCunt;
    bool                mDebug;
};

#else
#include <QMutex>
class zchxMulticastDataScoket : public QThread
{
    Q_OBJECT
public:
    explicit zchxMulticastDataScoket(const QString & ip, const  QString& host,
                                     int    port,
                                     const QNetworkInterface nif = QNetworkInterface(),
                                     QObject *parent = 0);

    virtual ~zchxMulticastDataScoket();
    void    setDebug(bool sts) {mDebug = sts;}
    void    writeData(const QByteArray &data);
    bool    isFine() const;
    void    setIsOver(bool sts) {mIsOver = true;}

protected:
    void    run();

signals:
    void signalSendRecvData(const QByteArray& data);
    void signalNoDataRecv();
private:
    QUdpSocket* init();
    virtual void processRecvData(const QByteArray& data);

private:
    QString             mHost;
    int                 mPort;
    QString             mLocalIP;
    QDateTime           mLastRecvTime;
    QNetworkInterface   mNIF;
    bool                mDebug;
    QByteArrayList          mSendDataList;
    QMutex              mMutex;
    bool                mIsOver;
};

#endif

#endif // ZCHXMULTICASTDATASOCKET_H
