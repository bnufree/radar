#include "up_video_pthread.h"
#include <QThread>
#include <QFile>
#include <QString>
#include <QDialog>
#include <QFileDialog>
#include <QDirIterator>

up_video_pthread::up_video_pthread(const QString& filename, QObject *parent) :
    mDir(filename),QThread(parent)
{

}

up_video_pthread::~up_video_pthread()
{
    if(this->isRunning())
    {
        this->quit();
    }
    this->terminate();
}
void up_video_pthread::run()
{
    readAllFile(mDir);
    exec();
}
//按顺序读取回波文件
void up_video_pthread::readAllFile(QString path)
{
    QDir dir(path);
    if(!dir.exists())
    {
        return;
    }

    //获取所选文件类型过滤器
    QStringList filters;
    filters<<QString("*.dat");

    while(dir.exists())
    {
        //定义迭代器并设置过滤器
        QDirIterator dir_iterator(path, filters, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
        QStringList string_list;

        while(dir_iterator.hasNext())
        {
            dir_iterator.next();
            QFileInfo file_info = dir_iterator.fileInfo();
            QString file_path = file_info.absoluteFilePath();
            string_list.append(file_path);
            QFile video_file(file_path);
            if(video_file.open(QIODevice::ReadOnly))
            {
                int a =17160;
                while (1)
                {
                    QByteArray video_array;
                    video_array = video_file.read(17160);
                    a = video_array.size();
                    if(a != 0)
                    {
                        emit sendFileData(video_array);
                        msleep(50);
                    }
                    if(video_file.atEnd()) break;
                }
                //关闭文件
                video_file.close();

            }
        }
    }

}
