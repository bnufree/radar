#ifndef COMMON_H
#define COMMON_H

#include "zchxcommon_global.h"
#include <QStringList>
#include <QNetworkInterface>
#include <QString>

class COMMONSHARED_EXPORT zchxCommonUtils
{

public:
    zchxCommonUtils();
    static QString time2String(qint64 time, bool secs);
    static QStringList   getAllIpv4List();
    static const char*   QString2Char(const QString& str);
    static QMap<QString, QNetworkInterface>  getNetIfMap();
};

#endif // COMMON_H
