#include "zchxradarchannelsettingdlg.h"
#include "ui_zchxradarchannelsettingdlg.h"

zchxRadarChannelSettingDlg::zchxRadarChannelSettingDlg(const zchxCommon::zchxRadarChannel& channel, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::zchxRadarChannelSettingDlg),
    mChannel(channel)
{
    ui->setupUi(this);    
    setAttribute(Qt::WA_DeleteOnClose);
    this->setWindowFlags(Qt::CustomizeWindowHint|Qt::WindowCloseButtonHint);
    ui->videoRecIPLlineEdit->setText(mChannel.video.ip);
    ui->videoRecPortSpinBox->setValue(mChannel.video.port);
    ui->controlIPLineEdit->setText(mChannel.report.ip);
    ui->controlPortSpinBox->setValue(mChannel.report.port);
    ui->heartIPLlineEdit->setText(mChannel.heart.ip);
    ui->heartPortSpinBox->setValue(mChannel.heart.port);
}

zchxRadarChannelSettingDlg::~zchxRadarChannelSettingDlg()
{
    delete ui;
}

void zchxRadarChannelSettingDlg::on_ok_clicked()
{
    mChannel.video.ip = ui->videoRecIPLlineEdit->text().trimmed();
    mChannel.video.port = ui->videoRecPortSpinBox->value();
    mChannel.report.ip = ui->controlIPLineEdit->text().trimmed();
    mChannel.report.port = ui->controlPortSpinBox->value();
    mChannel.heart.ip = ui->heartIPLlineEdit->text().trimmed();
    mChannel.heart.port = ui->heartPortSpinBox->value();
    emit signalNewSetting(mChannel.toJson());
    close();
}

void zchxRadarChannelSettingDlg::on_cancel_clicked()
{
    close();
}
