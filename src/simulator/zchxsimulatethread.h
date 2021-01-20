#ifndef ZCHXSIMULATETHREAD_H
#define ZCHXSIMULATETHREAD_H

#include <QThread>

class zchxSimulateThread : public QThread
{
    Q_OBJECT
public:
    explicit zchxSimulateThread(const QString& dirName, QObject *parent = 0);
    void setCancel(bool sts) {mCancelFlg = sts;}

protected:
    void run();

signals:
    void signalSendContents(const QByteArray& bytes, int len);
    void signalSimulationEnd();
public slots:

private:
    QString         mDirName;
    bool            mCancelFlg;
};

#endif // ZCHXSIMULATETHREAD_H
