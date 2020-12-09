#include "zchxradarbasesettingdlg.h"
#include "ui_zchxradarbasesettingdlg.h"

zchxRadarBaseSettingDlg::zchxRadarBaseSettingDlg(const zchxCommon::zchxRadarDeviceBaseSetting& setting,
                                                 QWidget *parent) :
    QDialog(parent),
    ui(new Ui::zchxRadarBaseSettingDlg),
    mSetting(setting)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::CustomizeWindowHint|Qt::WindowCloseButtonHint);
    setAttribute(Qt::WA_DeleteOnClose);
    ui->lon->setFocus();
    ui->lat->setValue(mSetting.center.lat);
    ui->lon->setValue(mSetting.center.lon);
    ui->name->setText(mSetting.name);
}

zchxRadarBaseSettingDlg::~zchxRadarBaseSettingDlg()
{
    delete ui;
}

void zchxRadarBaseSettingDlg::on_ok_clicked()
{
    mSetting.center.lat = ui->lat->value();
    mSetting.center.lon = ui->lon->value();
    mSetting.name = ui->name->text();
    emit signalnewSetting(mSetting.toJson());
    close();
}

void zchxRadarBaseSettingDlg::on_cancel_clicked()
{
    close();
}

void zchxRadarBaseSettingDlg::on_pos_from_ecdis_clicked()
{
    emit signalGetPosFromEcdis();
}

void zchxRadarBaseSettingDlg::slotSetPos(double lat, double lon)
{
    ui->lat->setValue(lat);
    ui->lon->setValue(lon);
}
