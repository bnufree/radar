#include "zchxregistorchecker.h"
#include <QDir>
#include <QProcess>
#include <QDebug>
#include <QSettings>
#include <QDateTime>


zchxRegistorChecker::zchxRegistorChecker(QObject *parent) : QObject(parent)
{
    mRegOrg = QString::fromUtf8("ZCHX");
    mRegApp = QString::fromUtf8("RadarSystem");
    mRegKey = QString::fromUtf8("Date");
}

bool zchxRegistorChecker::startCheck(const QString &key)
{
    //如果key为空，就是目标已经注册，现在只需要检查时间
    //如果Key不为空，就是第一次注册，需要将时间限制写入
    QString wkKey;
    QString errMsg;
    bool sts = false;

    if(key.isEmpty())
    {
        //获取本机已经注册的K值
        wkKey =  QString::fromUtf8(QByteArray::fromBase64(getRegedit(mRegKey).toUtf8()));
    } else
    {
        //添加当前时期
        wkKey =  QString::fromUtf8(QByteArray::fromBase64(key.toUtf8()));
//        qDebug()<<"orignal key:"<<wkKey;
        //检查序列号的生成时间
        QDateTime check_time = QDateTime::fromString(wkKey.right(14), "yyyyMMddhhmmss");
        if(!check_time.isValid())
        {
            errMsg = QString::fromUtf8("注册码错误1");
            goto FUNC_END;
        }
        if(check_time.secsTo(QDateTime::currentDateTime()) > 3600)
        {
            errMsg = QString::fromUtf8("注册码错误2");
            goto FUNC_END;
        }
        wkKey.remove(wkKey.size()-14, 14);
        wkKey.append(QString("").sprintf("%010d", QDateTime::currentDateTime().toTime_t()));
    }
    if(wkKey.isEmpty())
    {
        errMsg = QString::fromUtf8("系统未注册");
        goto FUNC_END;
    } else
    {
//        qDebug()<<"register info:"<<wkKey;
        QString cur_machine = getMachineCode();
        //检查注册码是否与本机的ID值相匹配
        //注册码格式=硬件信息+使用时限（10位数）+开始日期(时间戳10位)
        //将注册码格式进行解码
        QString hardware = wkKey.left(wkKey.size() - 20);
        QString range = wkKey.mid(hardware.size(), 10);
        QString start_day = wkKey.right(10);
        //    qDebug()<<hardware<<range<<start_day;
        if(hardware != cur_machine)
        {
            errMsg = QString::fromUtf8("注册码错误3");
            goto FUNC_END;
        }
        QDateTime start_time = QDateTime::fromTime_t(start_day.toUInt());
        if(!start_time.isValid())
        {
            errMsg = QString::fromUtf8("注册码错误4");
            goto FUNC_END;
        }
        if(range.isEmpty())
        {
            errMsg = QString::fromUtf8("注册码错误5");
            goto FUNC_END;
        }
        //检查是不是全是数字
        QRegExp checker("\\d{10}");
        if(!checker.exactMatch(range))
        {
            errMsg = QString::fromUtf8("注册码错误6");
            goto FUNC_END;
        }
        while (1) {
            if(range.size() == 0) break;
            if(range.left(1) == "0")
            {
                range.remove(0, 1);
            } else
            {
                break;
            }
        }

        if(range.isEmpty())
        {
            sts = true;
        } else if(start_time.addSecs(range.toUInt()) >= QDateTime::currentDateTime())
        {
            sts = true;
        } else
        {
            errMsg = QString::fromUtf8("注册码错误7");
            goto FUNC_END;
        }

        if(sts && !key.isEmpty())
        {
            removeRegedit(mRegKey);
            writeRegedit(mRegKey, wkKey.toUtf8().toBase64());
        }
    }

FUNC_END:
    if(errMsg.size() > 0)
    {
        qDebug()<<"error msg:"<<errMsg;
    }
    return sts;
}

QString zchxRegistorChecker::getCPUId() const
{
    QProcess process(0);
    process.setProgram("cmd");
    QStringList argument;
    argument<<"/c"<<"wmic cpu get processorid";
    process.setArguments(argument);
    process.start();
    process.waitForStarted(); //等待程序启动
    process.waitForFinished();//等待程序关闭
    QString temp = QString::fromLocal8Bit(process.readAllStandardOutput());
    temp.remove(QRegExp("[ \\r\\n]")).remove("ProcessorId");
    return temp;
}

QString zchxRegistorChecker::getDiskId() const
{
    QProcess process(0);
    process.setProgram("cmd");
    QStringList argument;
    argument<<"/c"<<"wmic diskdrive get serialnumber";
    process.setArguments(argument);
    process.start();
    process.waitForStarted(); //等待程序启动
    process.waitForFinished();//等待程序关闭
    QString temp = QString::fromLocal8Bit(process.readAllStandardOutput());
    temp.remove(QRegExp("[ \\r\\n]")).remove("SerialNumber");
    return temp;
}

QString zchxRegistorChecker::getMachineCode() const
{
    //获取本机的CPU信息+硬盘信息
    return getCPUId() + getDiskId();
}


void zchxRegistorChecker::writeRegedit(QString key, QString val)
{
    QSettings Setting(QSettings::NativeFormat, QSettings::SystemScope, mRegOrg, mRegApp);
    Setting.setValue(key, val);
}

QString zchxRegistorChecker::getRegedit(QString key)
{
    QSettings Setting(QSettings::NativeFormat, QSettings::SystemScope, mRegOrg, mRegApp);
    QString val = Setting.value(key).toString();
    return val;
}

void zchxRegistorChecker::removeRegedit(QString key)
{
    QSettings Setting(QSettings::NativeFormat, QSettings::SystemScope, mRegOrg, mRegApp);
    Setting.remove(key);
}


//void QVerificationThread::run()
//{
//    while (true) {
//        QString firstDate = getRegedit(mKey);
//        if(firstDate.isEmpty())
//        {
//            writeRegedit(mKey, QDate::currentDate().toString("yyyy-MM-dd").toUtf8().toBase64());
//            firstDate = getRegedit(mKey);
//        }
//        QDate startDate = QDate::fromString(QString::fromUtf8(QByteArray::fromBase64(firstDate.toUtf8())), "yyyy-MM-dd");
//        qDebug()<<"app start date:"<<startDate;
//        if(startDate.isValid())
//        {
//            int day = startDate.daysTo(QDate::currentDate());
//            int expired = 30 - day;
//            emit signalTrialExpired(expired);
//        } else
//        {
//            emit signalTrialExpired(-1);
//        }
//        sleep(60);
//    }

//}

