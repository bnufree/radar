#include "zchxsimulatethread.h"
#include <QDir>
#include <QDirIterator>
#include <QDebug>

zchxSimulateThread::zchxSimulateThread(const QString& dirName, QObject *parent)
    : QThread(parent)
    , mDirName(dirName)
    , mCancelFlg(false)
{

}

void zchxSimulateThread::run()
{
    QDir dir(mDirName);
    if(!dir.exists()) return;

    //获取所选文件类型过滤器
    QStringList filters;
    filters<<QString("*.dat");

    while(1)
    {
        if(mCancelFlg) break;
        //定义迭代器并设置过滤器
        QDirIterator dir_iterator(mDirName, filters, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
        while(dir_iterator.hasNext())
        {
            if(mCancelFlg) break;
            dir_iterator.next();
            QFileInfo file_info = dir_iterator.fileInfo();
            QString file_path = file_info.absoluteFilePath();
            qDebug()<<"now read file:"<<file_path;
            QFile file(file_path);
            if(!file.open(QIODevice::ReadOnly))
            {
                qDebug()<<"open file failed...";
                continue;
            }
            //文件格式是数据时间＋数据长度＋数据（qint64 + qint64 + data）
            QByteArray contents;
            qint64 last_send_time = 0;
            while (!mCancelFlg)
            {
                contents = file.read(sizeof(qint64));
                qint64 time = 0;
                memcpy(&time, contents.data(), sizeof(qint64));
                qint64 data_len = 0;
                contents = file.read(sizeof(qint64));
                memcpy(&data_len, contents.data(), sizeof(qint64));
                contents = file.read(data_len);
                if(last_send_time == 0)
                {
                    last_send_time = time;
                }
                int sub_time = time - last_send_time;
                msleep(sub_time);
                emit signalSendContents(contents, contents.size());
                last_send_time = time;

                if(file.atEnd()) break;
            }
            //关闭文件
            file.close();

        }


    }


}
