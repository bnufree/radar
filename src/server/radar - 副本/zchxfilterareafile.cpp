#include "zchxfilterareafile.h"
#include <QFile>
#include <QDebug>


zchxFilterAreaFile::zchxFilterAreaFile(const QString& fileName)
   : mFileName(fileName)
{
    init();
}

void zchxFilterAreaFile::init()
{
    QFile file(mFileName);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
        if(err.error == QJsonParseError::NoError)
        {
            //支持一个或者多个区域
            QJsonArray array;
            if(doc.isObject())
            {
                array.append(doc.object());
            } else if(doc.isArray())
            {
                array = doc.array();
            }
            if(array.size() == 0)
            {
                qDebug()<<"no filter area found in file."<<mFileName;
            } else
            {
                for(int i=0; i<array.size(); i++)
                {
                    zchxCommon::zchxFilterArea area(array[i].toObject());
                    qDebug()<<"filter:"<<area.id<<area.type<<area.name;
                    if(area.id >= 0)
                    {
                        if(mAreaMap.contains(area.id))
                        {
                            qDebug()<<"find multi area id "<<area.id<<mFileName;
                            continue;
                        }
                        mAreaMap[area.id] = area;
                    } else
                    {
                        qDebug()<<"find abnormal area id negiative now"<<area.id<<mFileName;
                    }

                }

                qDebug()<<"filter area found in file."<<mFileName<< "with size:"<<mAreaMap.size();

            }
        } else
        {
            qDebug()<<"parse json file error occured."<<err.error<<err.errorString()<<mFileName;
        }

        //开始进行编号
        file.close();

    } else
    {
        qDebug()<<"open file failed.."<<mFileName;
    }
}

bool zchxFilterAreaFile::updateFile()
{
    QFile file(mFileName);
    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QJsonArray array;
        foreach (zchxCommon::zchxFilterArea obj, mAreaMap) {
            array.append(obj.toJson());
        }
        QJsonDocument doc;
        doc.setArray(array);
        file.write(doc.toJson());
        file.close();

        return true;
    }

    return false;
}

bool zchxFilterAreaFile::removeArea(const QList<int> ids)
{
    foreach (int id, ids) {
        if(!mAreaMap.contains(id)) continue;
        mAreaMap.remove(id);
    }
    if(updateFile())
    {
        qDebug()<<"remove area ok"<<ids<<mFileName;
        return true;
    }

    qDebug()<<"remove area  error"<<ids<<mFileName;
    return false;
}

bool zchxFilterAreaFile::addArea(const zchxCommon::zchxfilterAreaList &list)
{
    //更新的情况
    foreach (zchxCommon::zchxFilterArea area, list) {
        int id = 1;
        if(area.id > 0 && mAreaMap.contains(area.id))
        {
            mAreaMap[area.id] = area;
            id = area.id;
        } else
        {
            while (mAreaMap.contains(id)) {
                id++;
            }
            mAreaMap[id] = area;
            mAreaMap[id].id = id;
        }
    }
    if(updateFile())
    {
        qDebug()<<"add area ok"<<mFileName;
        return true;
    }

    qDebug()<<"add area error"<<mFileName;
    return false;
}


zchxCommon::zchxfilterAreaList zchxFilterAreaFile::getFilterAreaList() const
{
    return zchxCommon::zchxfilterAreaList(mAreaMap.values());
}

QByteArray zchxFilterAreaFile::getFilterAreaByteArray() const
{
    QJsonArray array;
    foreach (zchxCommon::zchxFilterArea obj, mAreaMap) {
        array.append(obj.toJson());
    }
    QJsonDocument doc;
    doc.setArray(array);
    return doc.toJson();
}
