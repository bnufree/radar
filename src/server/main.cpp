#include <QCoreApplication>
#include <QDir>
#include <QMutex>
#include <QTextStream>
#include <QTextCodec>
#include <iostream>
#include "mainprocess.h"
#include <QProcess>

void logMessageOutputQt5(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    static QString log_file_name = "";
    static qint64 max = 10485760;//10 * 1024 * 1024;
    static QMutex mutex;
    static qint64 log_file_lenth = 0;
    mutex.lock();

    QString text;
    switch (type) {
    case QtDebugMsg:
        text = QString("Debug:");
        break;
    case QtWarningMsg:
        text = QString("Warning:");
        break;
    case QtCriticalMsg:
        text = QString("Critical:");
        break;
    case QtFatalMsg:
        text = QString("Fatal:");
        abort();
    default:
        break;
    }
    QString message = QString("[%1] %2 [%3] [%4] %5").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
            .arg(text).arg(context.file).arg(context.line).arg(msg);

    QString appName = QCoreApplication::applicationName();
    QString logPath = QCoreApplication::applicationDirPath() + QString("/log/") + appName;

    QDir dir(logPath);
    if(!dir.exists())
    {
        dir.mkpath(dir.path());
    } else
    {
    }
    if(log_file_name.isEmpty() || log_file_lenth > max)
    {
        //重新启动的情况,将日志目录下的文件删除,保留最近的文件
        {
            QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, QDir::Time);
            for(int i=5; i<list.size();i++)
            {
                QFileInfo info = list[i];
                QFile::remove(info.absoluteFilePath());
            }
        }
        log_file_lenth = 0;
        log_file_name = QString("%1/%2_%3.txt").arg(logPath).arg(appName).arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss"));
    }
    QFile file(log_file_name);
    if(file.open(QIODevice::ReadWrite | QIODevice::Append))
    {
        QTextStream text_stream(&file);
        text_stream << message << endl;
        file.flush();
        file.close();
        log_file_lenth = file.size();
    } else
    {
        std::cout <<"open file failed...........";
    }
    mutex.unlock();
    message = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss  ");
    std::cout << message.toStdString();
    std::cout << msg.toUtf8().toStdString() <<std::endl;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    QProcess p(0);
    p.start("cmd", QStringList()<<"/c"<<"chcp 65001");
    p.waitForStarted();
    p.waitForFinished();
    qInstallMessageHandler(logMessageOutputQt5);
    MainProc->start();
    return a.exec();
}
