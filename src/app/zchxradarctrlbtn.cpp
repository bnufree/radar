#include "zchxradarctrlbtn.h"
#include "zchxradaroptwidget.h"
#include "zchxdatadef.h"
#include <QDebug>

using namespace zchxCommon;
struct RadarCtrlDataRange
{
    int     min;
    int     max;
    bool    auto_ok;
    int     adjust_mode;
};

RadarCtrlDataRange ctrl_data_range[] = {
    {0, 0, false, MIDE_ADJUST_MODE},         //未知命令， 这里暂时不用
    {0, 1, false, SMALL_ADJUST_MODE},         //电源
    {0, 1, false, SMALL_ADJUST_MODE},         //扫描速度
    {0, 30, false, MIDE_ADJUST_MODE},        //高度
    {-180, 180, false, MIDE_ADJUST_MODE},    //方位角
    {0, 72707, false, LARGE_ADJUST_MODE},     //扫描半径
    {-1, 100, true, MIDE_ADJUST_MODE},      //增益
    {-1, 100, true, MIDE_ADJUST_MODE},      //海杂波
    {0, 100, false, MIDE_ADJUST_MODE},       //雨杂波
    {0, 2, false, SMALL_ADJUST_MODE},         //噪声抑制
    {-1, 100, true, MIDE_ADJUST_MODE},       //旁瓣抑制
    {0, 3, false, SMALL_ADJUST_MODE},         //抗干扰
    {0, 3, false, SMALL_ADJUST_MODE},         //本地抗干扰
    {0, 1, false, SMALL_ADJUST_MODE},         //目标扩展
    {0, 2, false, SMALL_ADJUST_MODE},         //目标推进
    {0, 3, false, SMALL_ADJUST_MODE}          //目标分离
};


zchxRadarCtrlBtn::zchxRadarCtrlBtn(QWidget *parent) : QPushButton(parent)
{
    mType = -1;
    mMin = -1;
    mMax = INT_MAX;
    mOptWidget = 0;
    mAutoAvailable = false;
    mAdjustMode = MIDE_ADJUST_MODE;
    connect(this, SIGNAL(clicked(bool)), this, SLOT(btnClicked()));
}

void zchxRadarCtrlBtn::setType(int type)
{
    mType = type;
    mMin = ctrl_data_range[mType].min;
    mMax = ctrl_data_range[mType].max;
    mAutoAvailable = ctrl_data_range[mType].auto_ok;
    mAdjustMode = ctrl_data_range[mType].adjust_mode;
}

void zchxRadarCtrlBtn::setValue(int value)
{
    if(mOptWidget) mOptWidget->setCurrentVal(value);
    QString text = QString::number(value);
    switch (mType) {
    case SCAN_SPEED:
        text = getScanSpeedString(value);
        break;
    case NOISE_REJECTION:
    case TARGET_BOOST:
        text = getHLOString(value);
        break;
    case INTERFERENCE_REJECTION:
    case LOCAL_INTERFERENCE_REJECTION:
    case TARGET_SEPARATION:
        text = getHMLOString(value);
        break;
    case TARGET_EXPANSION:
        text = getOpenString(value);
        break;
    case POWER:
        text = getPowerString(value);
        break;
    default:
        break;
    }
    if(mAutoAvailable && value == -1)
    {
        text = QStringLiteral("自动");
    }

    setText(text);
    mCur = value;
}

void zchxRadarCtrlBtn::btnClicked()
{
    if(mAdjustMode != SMALL_ADJUST_MODE)
    {
        if(!parentWidget()) return;
        if(!mOptWidget)
        {
            mOptWidget = new zchxRadarOptWidget();
            connect(mOptWidget, SIGNAL(signalClose()), this, SLOT(slotBtnClose()));
            connect(mOptWidget, SIGNAL(signalConfigChanged(int)), this, SLOT(slotConfigChanged(int)));
            mOptWidget->setStyleSheet(parentWidget()->styleSheet());
        }
        //将目标移动到当前位置下方
        mOptWidget->setAdjustMode(mAdjustMode);
        mOptWidget->setAutoBtnAvailable(mAutoAvailable);
        mOptWidget->setRange(mMin, mMax);
        mOptWidget->setCurrentVal(mCur);
        mOptWidget->setWidth(this->width());
        QRect rect(QPoint(0,0), mOptWidget->size());
        QPoint pos = parentWidget()->mapToGlobal(this->geometry().bottomLeft());
        rect.moveTopLeft(pos);
        mOptWidget->setGeometry(rect);
        mOptWidget->show();
    } else
    {
        if(mMax <= 0) return;
        slotConfigChanged((mCur + 1) % (mMax+1));
    }

}

void zchxRadarCtrlBtn::slotBtnClose()
{
    if(mOptWidget)
    {
        delete mOptWidget;
        mOptWidget = 0;
    }
}


void zchxRadarCtrlBtn::slotConfigChanged(int value)
{
    emit signalCtrlValChanged(mType, value);
}

QString zchxRadarCtrlBtn::getPowerString(int sts)
{
    if(sts == POWER_STANDBY) return QStringLiteral("待机");
    if(sts == POWER_TRANSMIT) return QStringLiteral("传输");
    if(sts == POWER_OFF) return QStringLiteral("断开");
    return QStringLiteral("未知");
}

QString zchxRadarCtrlBtn::getScanSpeedString(int sts)
{
    qDebug()<<"speed sts:"<<sts;
    if(sts == SPEED_NORMAL) return QStringLiteral("正常");
    if(sts == SPEED_FAST) return QStringLiteral("快速");
    return QStringLiteral("未知");

}

QString zchxRadarCtrlBtn::getHLOString(int sts)
{
    if(sts == HLO_LOW) return QStringLiteral("低");
    if(sts == HLO_HIGH) return QStringLiteral("高");
    if(sts == HLO_OFF) return QStringLiteral("关闭");
    return QStringLiteral("未知");
}

QString zchxRadarCtrlBtn::getHMLOString(int sts)
{
    if(sts == HMLO_LOW) return QStringLiteral("低");
    if(sts == HMLO_HIGH) return QStringLiteral("高");
    if(sts == HMLO_MIDDLE) return QStringLiteral("中");
    if(sts == HMLO_OFF) return QStringLiteral("关闭");
    return QStringLiteral("未知");
}

QString zchxRadarCtrlBtn::getOpenString(int sts)
{
    if(sts == OFF) return QStringLiteral("关闭");
    if(sts == ON) return QStringLiteral("开启");
    return QStringLiteral("未知");
}


