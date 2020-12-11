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
    qDebug()<<"start init filter area from file:"<<mFileName;
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QJsonParseError err;
        QByteArray bytes = file.readAll();
        if(bytes.size() > 0)
        {
            QJsonDocument doc = QJsonDocument::fromJson(bytes, &err);
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
                    qDebug()<<"filter area not found, pls check file...";
                } else
                {
                    for(int i=0; i<array.size(); i++)
                    {
                        zchxCommon::zchxFilterArea area(array[i].toObject());
                        qDebug()<<"filter area:"<<area.id<<area.type<<area.name;
                        if(area.id >= 0)
                        {
                            if(mAreaMap.contains(area.id))
                            {
                                qDebug()<<"multi filter area id:"<<area.id<<mFileName;
                                continue;
                            }
                            mAreaMap[area.id] = area;
                        } else
                        {
                            qDebug()<<"abnormal filter area id"<<area.id<<mFileName;
                        }

                    }

                    qDebug()<<"total filter area size:"<<mAreaMap.size();

                }
            } else
            {
                qDebug()<<"parse filter area json file abnormal end."<<" error code:"<<err.error<<err.errorString();
            }
        } else
        {
            qDebug()<<"filter area file is empty. filter area not set yet.";
        }

        //开始进行编号
        file.close();

    } else
    {
        qDebug()<<"open filter area file failed.";
    }
}

bool zchxFilterAreaFile::updateFile()
{
    bool sts = false;
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

        sts = true;
    }
    QString result = (sts == true ? "成功" : "失败");

    qDebug()<<QString("更新屏蔽区域到文件:%1 %2").arg(mFileName).arg(result);

    return sts;
}

bool zchxFilterAreaFile::removeArea(const QList<int> ids)
{
    qDebug()<<QString("开始删除屏蔽区域：")<<ids;
    foreach (int id, ids) {
        if(!mAreaMap.contains(id)) continue;
        mAreaMap.remove(id);
    }
    if(updateFile())
    {
        qDebug()<<QString("删除屏蔽区域成功");
        return true;
    }

    qDebug()<<QString("删除屏蔽区域失败");
    return false;
}

bool zchxFilterAreaFile::addArea(const zchxCommon::zchxfilterAreaList &list)
{
    qDebug()<<QString("开始更新/添加屏蔽区域");
    //更新的情况
    foreach (zchxCommon::zchxFilterArea area, list) {
        int id = 1;
        if(area.id > 0 && mAreaMap.contains(area.id))
        {
            qDebug()<<QString("屏蔽区域已经存在，更新对应ID:")<<area.id;
            mAreaMap[area.id] = area;
            id = area.id;
        } else
        {
            while (mAreaMap.contains(id)) {
                id++;
            }
            mAreaMap[id] = area;
            mAreaMap[id].id = id;
            qDebug()<<QString("屏蔽区域不存在，新添加ID:")<<id;
        }
    }
    if(updateFile())
    {
        qDebug()<<QString("更新/添加屏蔽区域成功");
        return true;
    }

    qDebug()<<QString("更新/添加屏蔽区域失败");
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
