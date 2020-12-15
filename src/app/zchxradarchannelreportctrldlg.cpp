#include "zchxradarchannelreportctrldlg.h"
#include "ui_zchxradarchannelreportctrldlg.h"
#include <QDebug>
#include <QTimer>

#define  MOD(val) (((val) < 0) ? ((val) + 360) :((val) >= 3600 ? ((val) - 3600) : (val)))

using namespace zchxCommon;
zchxRadarChannelReportCtrlDlg::zchxRadarChannelReportCtrlDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::zchxRadarChannelReportCtrlDlg)
{
    ui->setupUi(this);
    ui->antenna_btn->setType(ANTENNA_HEIGHT);
    ui->bearing_btn->setType(BEARING_ALIGNMENT);
    ui->range_btn->setType(RANG);
    ui->gain_btn->setType(GAIN);
    ui->rain_culter_btn->setType(RAIN_CLUTTER);
    ui->sea_culter_btn->setType(SEA_CLUTTER);
    ui->sidelobe_btn->setType(SIDE_LOBE_SUPPRESSION);
    ui->scan_seed_btn->setType(SCAN_SPEED);
    ui->noise_injection_btn->setType(NOISE_REJECTION);
    ui->local_injection_btn->setType(LOCAL_INTERFERENCE_REJECTION);
    ui->target_boost_btn->setType(TARGET_BOOST);
    ui->target_separat_btn->setType(TARGET_SEPARATION);
    ui->target_expansion_btn->setType(TARGET_EXPANSION);
    ui->injection_btn->setType(INTERFERENCE_REJECTION);
    ui->power_statsu_btn->setType(POWER);
    appendBtn(ui->antenna_btn);
    appendBtn(ui->bearing_btn);
    appendBtn(ui->range_btn);
    appendBtn(ui->gain_btn);
    appendBtn(ui->rain_culter_btn);
    appendBtn(ui->sea_culter_btn);
    appendBtn(ui->sidelobe_btn);
    appendBtn(ui->scan_seed_btn);
    appendBtn(ui->noise_injection_btn);
    appendBtn(ui->local_injection_btn);
    appendBtn(ui->target_boost_btn);
    appendBtn(ui->target_separat_btn);
    appendBtn(ui->target_expansion_btn);
    appendBtn(ui->injection_btn);
    appendBtn(ui->power_statsu_btn);
    connect(ui->fan1, &QCheckBox::toggled, [=](bool sts){
        ui->angle1->setEnabled(sts);
        ui->width1->setEnabled(sts);
    });
    connect(ui->fan4, &QCheckBox::toggled, [=](bool sts){
        ui->angle4->setEnabled(sts);
        ui->width4->setEnabled(sts);
    });
    connect(ui->fan2, &QCheckBox::toggled, [=](bool sts){
        ui->angle2->setEnabled(sts);
        ui->width2->setEnabled(sts);
    });
    connect(ui->fan3, &QCheckBox::toggled, [=](bool sts){
        ui->angle3->setEnabled(sts);
        ui->width3->setEnabled(sts);
    });

    ui->fan1->setChecked(false);
    ui->fan2->setChecked(false);
    ui->fan3->setChecked(false);
    ui->fan4->setChecked(false);

    ui->video_output_start_btn->setProperty("STATUS", false);
    ui->close_radar_btn->setProperty("STATUS", true);
}

void zchxRadarChannelReportCtrlDlg::setControlFrameVisible(bool sts)
{
    ui->common_frame->setVisible(sts);
}

void zchxRadarChannelReportCtrlDlg::setFanFrameVisible(bool sts)
{
    ui->skipVideoframe->setVisible(sts);
}


zchxRadarChannelReportCtrlDlg::~zchxRadarChannelReportCtrlDlg()
{
    delete ui;
}


void zchxRadarChannelReportCtrlDlg::appendBtn(zchxRadarCtrlBtn *btn)
{
    if(!btn) return;
    mBtnList.insert(btn->getType(), btn);
    connect(btn, &zchxRadarCtrlBtn::signalCtrlValChanged, [=](int type, int val){
        emit signalCtrlValChanged(type, QJsonValue(val));
    });
}


void zchxRadarChannelReportCtrlDlg::setReportInfo(const zchxCommon::zchxRadarReportDataList& list, int radar, int channel)
{
    foreach (zchxCommon::zchxRadarReportData ctrl, list) {
        if(ctrl.type < zchxCommon::FAN_IGNORE)
        {
            zchxRadarCtrlBtn* btn = mBtnList.value(ctrl.type, 0);
            if(btn)
            {
                btn->setValue(ctrl.jsval.toInt());
            }
        } else
        {
            qDebug()<<ctrl.type<<ctrl.jsval;
            if(ctrl.type == zchxCommon::OUTPUT_DATA)
            {
                if(ctrl.jsval.toBool())
                {
                    ui->video_output_start_btn->setText(QString::fromUtf8("停止回波打印"));
                } else
                {
                    ui->video_output_start_btn->setText(QString::fromUtf8("开始回波打印"));
                }
                ui->video_output_start_btn->setProperty("STATUS", ctrl.jsval.toBool());

            } else if(ctrl.type == zchxCommon::OPEN)
            {
                if(ctrl.jsval.toBool())
                {
                    ui->close_radar_btn->setText(QString::fromUtf8("关闭雷达"));
                } else
                {
                    ui->close_radar_btn->setText(QString::fromUtf8("打开雷达"));
                }

                ui->close_radar_btn->setProperty("STATUS", ctrl.jsval.toBool());
            }
        }
    }

    mRadarID = radar;
    mChannelID = channel;
}

void zchxRadarChannelReportCtrlDlg::setRadarType(int type)
{
    switch (type) {
    case RADAR_BR24:
        ui->radar_type->setText(QString::fromUtf8("BR24"));
        break;
    case RADAR_3G:
        ui->radar_type->setText(QString::fromUtf8("3G"));
        break;
    case RADAR_4G:
        ui->radar_type->setText(QString::fromUtf8("4G"));
        break;
    case RADAR_6G:
        ui->radar_type->setText(QString::fromUtf8("6G"));
        break;
    default:
        ui->radar_type->setText(QString::fromUtf8("未知"));
        break;
    }
}

void zchxRadarChannelReportCtrlDlg::updateRangefactor(double radius, double factor)
{
    ui->radius->setText(QString::number(radius, 'f', 0));
    ui->factor->setText(QString::number(factor, 'f', 2));
}


void zchxRadarChannelReportCtrlDlg::on_openOrCloseRadarBtn_clicked()
{

}

void zchxRadarChannelReportCtrlDlg::on_fan1_clicked(bool checked)
{
    QJsonArray array;
    array.append(0);
    array.append(checked);
    emit signalCtrlValChanged(zchxCommon::FAN_IGNORE, QJsonValue(array));
}

void zchxRadarChannelReportCtrlDlg::on_fan2_clicked(bool checked)
{
    QJsonArray array;
    array.append(1);
    array.append(checked);
    emit signalCtrlValChanged(zchxCommon::FAN_IGNORE, QJsonValue(array));
}

void zchxRadarChannelReportCtrlDlg::on_fan3_clicked(bool checked)
{
    QJsonArray array;
    array.append(2);
    array.append(checked);
    emit signalCtrlValChanged(zchxCommon::FAN_IGNORE, QJsonValue(array));
}

void zchxRadarChannelReportCtrlDlg::on_fan4_clicked(bool checked)
{
    QJsonArray array;
    array.append(3);
    array.append(checked);
    emit signalCtrlValChanged(zchxCommon::FAN_IGNORE, QJsonValue(array));
}

void zchxRadarChannelReportCtrlDlg::on_angle1_valueChanged(int arg1)
{
    QJsonArray array;
    array.append(0);
    array.append(true);
    array.append(MOD(arg1 - ui->width1->value() / 2) * 2560 % 65535);
    array.append(MOD(arg1 + ui->width1->value() / 2) * 2560 % 65535);
    qDebug()<<array;
    emit signalCtrlValChanged(zchxCommon::FAN_IGNORE, QJsonValue(array));
}

void zchxRadarChannelReportCtrlDlg::on_width1_valueChanged(int arg1)
{
    QJsonArray array;
    array.append(0);
    array.append(true);
    array.append(MOD(ui->angle1->value() - arg1 / 2) * 2560 % 65535);
    array.append(MOD(ui->angle1->value() + arg1 / 2) * 2560 % 65535);
    qDebug()<<array;
    emit signalCtrlValChanged(zchxCommon::FAN_IGNORE, QJsonValue(array));
}

void zchxRadarChannelReportCtrlDlg::on_angle2_valueChanged(int arg1)
{

    QJsonArray array;
    array.append(1);
    array.append(true);
    array.append(MOD(arg1 - ui->width2->value() / 2) * 2560 % 65535);
    array.append(MOD(arg1 + ui->width2->value() / 2) * 2560 % 65535);
    emit signalCtrlValChanged(zchxCommon::FAN_IGNORE, QJsonValue(array));
}

void zchxRadarChannelReportCtrlDlg::on_width2_valueChanged(int arg1)
{
    QJsonArray array;
    array.append(1);
    array.append(true);
    array.append(MOD(ui->angle2->value() - arg1 / 2) * 2560 % 65535);
    array.append(MOD(ui->angle2->value() + arg1 / 2) * 2560 % 65535);
    emit signalCtrlValChanged(zchxCommon::FAN_IGNORE, QJsonValue(array));
}

void zchxRadarChannelReportCtrlDlg::on_angle3_valueChanged(int arg1)
{
    QJsonArray array;
    array.append(2);
    array.append(true);
    array.append(MOD(arg1 - ui->width3->value() / 2) * 2560 % 65535);
    array.append(MOD(arg1 + ui->width3->value() / 2) * 2560 % 65535);
    emit signalCtrlValChanged(zchxCommon::FAN_IGNORE, QJsonValue(array));
}

void zchxRadarChannelReportCtrlDlg::on_width3_valueChanged(int arg1)
{
    QJsonArray array;
    array.append(2);
    array.append(true);
    array.append(MOD(ui->angle3->value() - arg1 / 2) * 2560 % 65535);
    array.append(MOD(ui->angle3->value() + arg1 / 2) * 2560 % 65535);
    emit signalCtrlValChanged(zchxCommon::FAN_IGNORE, QJsonValue(array));
}

void zchxRadarChannelReportCtrlDlg::on_angle4_valueChanged(int arg1)
{

    QJsonArray array;
    array.append(3);
    array.append(true);
    array.append(MOD(arg1 - ui->width4->value() / 2) * 2560 % 65535);
    array.append(MOD(arg1 + ui->width4->value() / 2) * 2560 % 65535);
    emit signalCtrlValChanged(zchxCommon::FAN_IGNORE, QJsonValue(array));
}

void zchxRadarChannelReportCtrlDlg::on_width4_valueChanged(int arg1)
{
    QJsonArray array;
    array.append(3);
    array.append(true);
    array.append(MOD(ui->angle4->value() - arg1 / 2) * 2560 % 65535);
    array.append(MOD(ui->angle4->value() + arg1 / 2) * 2560 % 65535);
    emit signalCtrlValChanged(zchxCommon::FAN_IGNORE, QJsonValue(array));
}

void zchxRadarChannelReportCtrlDlg::on_resetRadar_clicked()
{
    emit signalCtrlValChanged(zchxCommon::RESET, QJsonValue());
}

void zchxRadarChannelReportCtrlDlg::on_video_output_start_btn_clicked()
{

    emit signalCtrlValChanged(zchxCommon::OUTPUT_DATA, !(ui->video_output_start_btn->property("STATUS").toBool()));
    ui->video_output_start_btn->setEnabled(false);
    QTimer::singleShot(3000, this, [=](){
        ui->video_output_start_btn->setEnabled(true);
    });
}

void zchxRadarChannelReportCtrlDlg::on_close_radar_btn_clicked()
{
    emit signalCtrlValChanged(zchxCommon::OPEN, !(ui->close_radar_btn->property("STATUS").toBool()));
    ui->close_radar_btn->setEnabled(false);
    QTimer::singleShot(3000, this, [=](){
        ui->close_radar_btn->setEnabled(true);
    });
}
