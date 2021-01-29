#ifndef ZCHXRADAROUTPUTDATAMGR_H
#define ZCHXRADAROUTPUTDATAMGR_H

#include <QObject>
#include "zchxradarcommon.h"
#include "zchxdatadef.h"

class zchxRadarMergeThread;
class zchxRadarOutputDataMgr : public QObject
{
    Q_OBJECT
public:
    explicit zchxRadarOutputDataMgr( QObject *parent = 0);
    ~zchxRadarOutputDataMgr();
    void     updateVideoImage(int channel, const zchxRadarVideoSingleImage& img);
    void     updateTracks(int channel, const zchxRadarSurfaceTrack& track);
    void     updateChannelReport(int channel, const zchxCommon::zchxRadarChannelReport& report);
    void     updateRadarType(int channel, int type, const QString &radarID);
    void     updateRadarRadiusAndFactor(int channel, double radius, double factor, const QString &radarID);
    void     updateDelNodeLog(int channel, const QByteArray& log);

signals:

private:
    QMap<int, zchxRadarVideoSingleImage>                mVideoImageMap;
    QMap<int, zchxRadarSurfaceTrack>                    mTrackMap;
    QMap<int, zchxCommon::zchxRadarChannelReport>       mChannelReport;
    zchxRadarMergeThread*                   mMergeThread;
};

#endif // ZCHXRADAROUTPUTDATAMGR_H
