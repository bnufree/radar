#include "zchxradarvideowinrarthread.h"
#include <QProcess>
#include <QFileInfo>
#include <QDateTime>
#include <QDebug>
#include "private/qzipwriter_p.h"

zchxRadarVideoWinRarThread::zchxRadarVideoWinRarThread(const QStringList& fileList, QObject *parent) : QThread(parent)
{
    mFileList.append(fileList);
}

void zchxRadarVideoWinRarThread::run()
{
#ifdef Q_OS_WIN
    if(mFileList.size() == 0) return;
    //获取文件目录
    QString rar_path = "C:/Program Files/WinRAR";
    rar_path.replace("\\", "/");
    if(rar_path.right(1) == "/") rar_path.remove(rar_path.size()-1, 1);
    QString first_file = mFileList.first();
    QFileInfo info(first_file);
    QString path = info.absolutePath();
    QString exe_name = QString("%1/Rar.exe").arg(rar_path);
//    QString result_name = QString("%1/%2.zip").arg(path).arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"));
    QString result_name = QString("%2.zip").arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"));
    QString list_name = QString("%1/%2.lst").arg(path).arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"));
    QFile tempFile(list_name);
    if(tempFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        foreach (QString file, mFileList) {
            QFileInfo now(file);
            tempFile.write(now.absoluteFilePath().toUtf8());
            tempFile.write("\n");
        }
        tempFile.close();
        QString cmd = QString("\"%1\" a -ep %2 @%3").arg(exe_name).arg(result_name).arg(list_name);
        QString bat_file = QString("%1/rar.bat").arg(path);
        QFile bat(bat_file);
        if(bat.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            bat.write(cmd.toUtf8().data());
            bat.close();
        }
        //开始执行压缩命令
        QProcess p(0);
        p.setWorkingDirectory(path);
        p.start(bat_file);
//        p.start("cmd", QStringList()<<"/c"<<QString("\"%1\" a %2 @%3").arg(exe_name).arg(result_name).arg(list_name));
//        p.start(exe_name, QStringList()<<QString(" a %1 @%2").arg(result_name).arg(list_name));
        qDebug()<<p.arguments().join(" ");
        p.waitForStarted();
        qDebug()<<p.errorString();
        p.waitForFinished();
        qDebug()<<p.errorString();
    }
    //删除中间的临时文件
    mFileList.append(list_name);
    foreach (QString st, mFileList) {
        QFile tempFile(st);
        tempFile.remove();
    }
#endif
}


