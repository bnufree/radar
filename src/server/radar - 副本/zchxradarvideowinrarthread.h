#ifndef ZCHXRADARVIDEOWINRARTHREAD_H
#define ZCHXRADARVIDEOWINRARTHREAD_H

#include <QThread>
class zchxRadarVideoWinRarThread : public QThread
{
    Q_OBJECT
public:
    explicit zchxRadarVideoWinRarThread(const QStringList& fileList, QObject *parent = 0);
    void run();

signals:

public slots:

private:
    QStringList         mFileList;
};

#endif // ZCHXRADARVIDEOWINRARTHREAD_H
