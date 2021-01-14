#ifndef ZCHXRECTEXTRACTIONTHREAD_H
#define ZCHXRECTEXTRACTIONTHREAD_H

#include <QObject>
#include <QDateTime>
#include <QMutex>
#include "zchxradarcommon.h"
#include "zchxdatadef.h"

typedef     QList<Latlon>       zchxLatlonList;


class zchxRadarRectExtraction : public QObject
{
    Q_OBJECT
public:
    explicit zchxRadarRectExtraction(const zchxVideoParserSettings& settings, QObject *parent = 0);
    ~zchxRadarRectExtraction();
    zchxVideoParserSettings getParseSettings() const {return mSettings;}
    bool  extractRectFromVideoSrcImg(zchxRadarRectDefList& list,  QImage& filterImg, const videoParseData& src);

protected:
    bool  isVideoPolygonAvailable(const QPolygonF& poly);


private:
    void  initFilterAreaInMector();

private:
    zchxVideoParserSettings         mSettings;
    QList<QPolygonF>                mOutFilterAreaMercatorList;
    QList<QPolygonF>                mInFilterAreaMercatorList;
    QList<zchxRadarRectDefList>     mDrawVideoList;
};

#endif // ZCHXRECTEXTRACTIONTHREAD_H
