#include "zchxcommonutils.h"
#include <QDateTime>
#include <QNetworkInterface>


zchxCommonUtils::zchxCommonUtils()
{
}


QString zchxCommonUtils::time2String(qint64 time, bool secs)
{
    if(secs) return QDateTime::fromMSecsSinceEpoch(time).toString("yyyy-MM-dd hh:mm:ss");
    return QDateTime::fromMSecsSinceEpoch(time).toString("yyyy-MM-dd");
}
QMap<QString, QNetworkInterface>  zchxCommonUtils::getNetIfMap()
{
    QMap<QString, QNetworkInterface>  if_map;
    foreach (QNetworkInterface _if, QNetworkInterface::allInterfaces())
    {
        foreach(QHostAddress ip, _if.allAddresses())
        {
            if(ip.protocol() != QAbstractSocket::IPv4Protocol) continue;
            QString ip_str = ip.toString();
            if(ip_str.startsWith("127.")) continue;
            if_map.insert(ip_str, _if);
        }
    }

    return if_map;
}

QStringList zchxCommonUtils::getAllIpv4List()
{
    QStringList list;
    //获取当前本机的ipv4的ip;
    //获取所有网络接口的列表
    foreach (QNetworkInterface netInterface, QNetworkInterface::allInterfaces())
    {
        QList<QNetworkAddressEntry> entryList = netInterface.addressEntries();
        foreach(QNetworkAddressEntry entry, entryList)
        {
            QHostAddress ip = entry.ip();
            if(ip.protocol() != QAbstractSocket::IPv4Protocol) continue;
            QString ip_str = ip.toString();
            if(ip_str.startsWith("127.")) continue;
//            if(ip_str.startsWith("169.254")) continue;
            list.append(ip_str);
        }
    }

    return list;
}

const char* zchxCommonUtils::QString2Char(const QString &str)
{
    QByteArray bytes = str.toLocal8Bit();
    char* dest = new char[bytes.size()+1];
    memcpy(dest, bytes.data(), bytes.size());
    dest[bytes.size()] = '\0';
    return dest;
}
