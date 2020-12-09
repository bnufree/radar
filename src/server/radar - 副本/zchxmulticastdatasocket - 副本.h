#ifndef ZCHXMULTICASTDATASOCKET_H
#define ZCHXMULTICASTDATASOCKET_H

#include <QObject>
#include <QUdpSocket>
#include <QTimer>
#include <QDateTime>

enum DataRecvMode{
    ModeAsyncRecv = 0,  //默认是异步接收
    ModeSyncRecv,       //同步接收
};

#define         Video_Tag           "Video Data"
#define         Heart_Tag           "Heart Data"
#define         Report_Tag          "Report Data"

class zchxMulticastDataScoket : public QObject
{
    Q_OBJECT
public:
    explicit zchxMulticastDataScoket(const  QString& host,
                                     int    port,
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
private slots:
    virtual void init();

signals:
    void signalSendRecvData(const QByteArray& data);
public slots:
    virtual void slotReadyReadMulticastData();
    virtual void slotDisplayUdpReportError(QAbstractSocket::SocketError);
    void    writeDatagram(const QByteArray& data);
    void    slotCheckDataRecv();

private:
    QString             mHost;
    int                 mPort;
    QUdpSocket          *mSocket;
    QStringList         mLocalIPList;
    bool                mInit;
    bool                mIsOk;
    QDateTime           mLastRecvTime;
//    QTimer*             mSwapIPTimer;
    int                 mErrCunt;
    QMap<QString, QNetworkInterface>    mNetIFMap;
    QList<QNetworkInterface>            mIfList;
    int                                 mIfIndex;
};



#endif // ZCHXMULTICASTDATASOCKET_H
