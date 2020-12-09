#ifndef UP_VIDEO_PTHREAD_H
#define UP_VIDEO_PTHREAD_H

#include <QThread>

class up_video_pthread : public QThread
{
    Q_OBJECT
public:
     up_video_pthread(const QString& dir, QObject *parent = 0);
     ~up_video_pthread();
     void run();
     void readAllFile(QString);


signals:
     void sendFileData(QByteArray);//导入回波数据

public slots:

private:
     QString        mDir;
};

#endif // UP_VIDEO_PTHREAD_H
