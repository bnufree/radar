#ifndef ZCHXDATAOUTPUTSERVERTHREAD_H
#define ZCHXDATAOUTPUTSERVERTHREAD_H

#include "zchxdatathread.h"
#include <QMutex>

typedef  QByteArrayList         zchxSendTask;
typedef  QList<zchxSendTask>  zchxSendTaskList;

class zchxDataOutputServerThread : public zchxDataThread
{
    Q_OBJECT
public:
    explicit zchxDataOutputServerThread(void* ctx, int port, QObject *parent = 0);
    ~zchxDataOutputServerThread();
    void     run();
    void     appendTopic(const QString& topic) {if(!mTopicList.contains(topic)) mTopicList.append(topic);}
    QStringList topic() const {return mTopicList;}
    void     removeTopic(const QString& topic) {mTopicList.removeOne(topic);}
private:

public slots:
    void    slotRecvContents(const QByteArray& content, const QString& topic);
private:
    zchxSendTaskList        mSendContentList;
    QMutex                  mMutex;
    QStringList             mTopicList;
};

#endif // ZCHXDATAOUTPUTSERVERTHREAD_H
