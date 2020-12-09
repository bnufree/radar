#ifndef ZCHXBASEDATA_H
#define ZCHXBASEDATA_H

#include <QObject>
#include <QVariant>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QDateTime>
#include <QColor>
#include "zchxcommon_global.h"

//数据列表的通用模板
struct COMMONSHARED_EXPORT zchxData
{
public:
    zchxData() {}
    zchxData(const QJsonObject& obj) {Q_UNUSED(obj)}
    zchxData(const QJsonArray& array) {Q_UNUSED(array)}
    zchxData(const QStringList& list) {Q_UNUSED(list)}

    virtual QJsonValue toJson() const {return QJsonObject();}
private:

};

template <typename T>
class zchxDataList:public QList<T>
{
public:
    inline  zchxDataList() : QList<T>()  { }
    inline  zchxDataList(const T& val) : QList<T>() { this->append(val);}
    inline  zchxDataList(const QList<T>& list) : QList<T>(list) {}
    zchxDataList(const QJsonArray& array) : QList<T>()
    {
        this->clear();
        for(int i=0; i<array.size(); i++)
        {
            QJsonValue val = array.at(i);
            if(val.isObject())
            {
                this->append(T(val.toObject()));
            } else if(val.isArray())
            {
                this->append(T(val.toArray()));
            }
        }
    }
    QJsonValue toJson() const
    {
        QJsonArray array;
        for(int i=0; i<this->size(); i++)
        {
            array.append(this->at(i).toJson());
        }
        return array;
    }
private:

};

#endif // ZCHXBASEDATA_H
