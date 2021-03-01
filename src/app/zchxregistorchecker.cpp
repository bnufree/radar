#include "zchxregistorchecker.h"
#include <QDir>
#include <QProcess>
#include <QDebug>
#include <QSettings>
#include <QDate>


zchxRegistorChecker::zchxRegistorChecker(QObject *parent) : QObject(parent)
{
    mRegOrg = QString::fromUtf8("ZCHX");
    mRegApp = QString::fromUtf8("RadarSystem");
    mRegKey = QString::fromUtf8("Date");
}

bool zchxRegistorChecker::startCheck(const QString &key)
{
    QString wkKey = key;
    if(wkKey.isEmpty())
    {
        //获取本机已经注册的K值
        wkKey = getRegedit(mRegKey);
    }
    if(wkKey.isEmpty()) return false;

    QString cur_machine = getMachineCode();
    //检查注册码是否与本机的ID值相匹配
    //注册码格式=硬件信息+使用时限（四位数）+开始日期(8)
    //将注册码格式进行解码
    wkKey = QString::fromUtf8(QByteArray::fromBase64(wkKey.toUtf8()));
    QString hardware = wkKey.left(wkKey.size() - 12);
    QString range = wkKey.mid(hardware.size(), 4);
    QString start_day = wkKey.right(8);
    qDebug()<<hardware<<range<<start_day;
    if(hardware != cur_machine) return false;
    QDate start_date = QDate::fromString(start_day, "yyyyMMdd");
    if(!start_date.isValid()) return false;
    if(range.isEmpty()) return false;
    //检查是不是全是数字
    QRegExp checker("\\d{4}");
    if(!checker.exactMatch(range)) return false;
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
    bool sts = false;
    if(range.isEmpty())
    {
        sts = true;
    } else if(start_date.addDays(range.toInt()) >= QDate::currentDate())
    {
        sts = true;
    }

    if(sts && !key.isEmpty())
    {
        writeRegedit(mRegKey, key);
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

