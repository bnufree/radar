#include "zchxradarui.h"
#include "ui_zchxradarui.h"
#include "zchxradarchannelreportctrldlg.h"
#include "zchxvideocolorsettingwidget.h"
#include <QMessageBox>

zchxRadarUi::zchxRadarUi(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::zchxRadarUi)
{
    ui->setupUi(this);
    ui->control_tab_layout->insertWidget(0, mReportDlg = new zchxRadarChannelReportCtrlDlg(this));
    ui->advance_tab_layout->insertWidget(0, mFanCtrlDlg = new zchxRadarChannelReportCtrlDlg(this));
    mReportDlg->setFanFrameVisible(false);
    mFanCtrlDlg->setControlFrameVisible(false);
    ui->target_area_filter_chk->setChecked(true);
    ui->target_length_filter_chk->setChecked(true);
    ui->video_amp_chk->setChecked(true);
    ui->target_area_filter_chk->setEnabled(false);
    ui->target_length_filter_chk->setEnabled(false);
    ui->video_amp_chk->setEnabled(false);
    ui->tab->setCurrentIndex(0);
}

zchxRadarUi::~zchxRadarUi()
{
    delete ui;
}

void zchxRadarUi::setChannelNum(int id)
{
    ui->channel_label->setText(QString::fromUtf8("通道%1").arg(id));
}

void zchxRadarUi::setRadarCenter(double lat, double lon)
{
    ui->lat->setValue(lat);
    ui->lon->setValue(lon);
}

void zchxRadarUi::setRadarDev(const zchxCommon::zchxRadarDevice &dev)
{
    mDev = dev;
    mReportDlg->setRadarID(dev.base.id);
    mFanCtrlDlg->setRadarID(dev.base.id);
    int id = -1;
    if(mDev.channel_list.size() > 0)
    {
        int id = mDev.channel_list.first().id;
    }
    mReportDlg->setChannelID(id);
    mReportDlg->setChannelID(id);

    ui->lat->setValue(dev.base.center.lat);
    ui->lon->setValue(dev.base.center.lon);
    ui->name->setText(dev.base.name);
    if(dev.channel_list.size() > 0)
    {
        ui->videoRecIPLlineEdit->setText(dev.channel_list.first().video.ip);
        ui->videoRecPortSpinBox->setValue(dev.channel_list.first().video.port);
        ui->controlIPLineEdit->setText(dev.channel_list.first().report.ip);
        ui->controlPortSpinBox->setValue(dev.channel_list.first().report.port);
        ui->heartIPLlineEdit->setText(dev.channel_list.first().heart.ip);
        ui->heartPortSpinBox->setValue(dev.channel_list.first().heart.port);
    }

    ui->target_length_filter_chk->setChecked(dev.parse_param.lenth.used);
    ui->target_length_min->setValue(dev.parse_param.lenth.min);
    ui->target_length_max->setValue(dev.parse_param.lenth.max);
    ui->target_area_filter_chk->setChecked(dev.parse_param.area.used);
    ui->target_area_min->setValue(dev.parse_param.area.min);
    ui->target_area_max->setValue(dev.parse_param.area.max);
    ui->video_amp_chk->setChecked(dev.parse_param.amp.used);
    ui->video_amplitude_min->setValue(dev.parse_param.amp.min);
    ui->video_amplitude_max->setValue(dev.parse_param.amp.max);
    ui->clear_track_time->setValue(dev.parse_param.clear_target_time);
    ui->max_history->setValue(dev.parse_param.max_history_num);
    ui->video_or_count->setValue(dev.parse_param.video_overlap_cnt);
    ui->predictionWidth->setValue(dev.parse_param.prediction_width);
    ui->targetMaxSpeedSpinBox->setValue(dev.parse_param.max_target_speed);
    ui->scanCycleTime->setValue(dev.parse_param.scan_time);
    ui->target_confirm_spinBox->setValue(dev.parse_param.confirm_target_cnt);
    ui->move_target_min_speed_spinBox->setValue(dev.parse_param.output_target_min_speed);
}

void zchxRadarUi::on_ok_parse_clicked()
{
    mDev.parse_param.lenth.used = ui->target_length_filter_chk->isChecked();
    mDev.parse_param.lenth.min = ui->target_length_min->value();
    mDev.parse_param.lenth.max = ui->target_length_max->value();
    mDev.parse_param.amp.used = ui->video_amp_chk->isChecked();
    mDev.parse_param.amp.min = ui->video_amplitude_min->value();
    mDev.parse_param.amp.max = ui->video_amplitude_max->value();
    mDev.parse_param.area.used = ui->target_area_filter_chk->isChecked();
    mDev.parse_param.area.min = ui->target_area_min->value();
    mDev.parse_param.area.max = ui->target_area_max->value();

    mDev.parse_param.clear_target_time = ui->clear_track_time->value();
    mDev.parse_param.confirm_target_cnt = ui->target_confirm_spinBox->value();
    mDev.parse_param.max_history_num = ui->max_history->value();
    mDev.parse_param.max_target_speed = ui->targetMaxSpeedSpinBox->value();
    mDev.parse_param.output_target_min_speed = ui->move_target_min_speed_spinBox->value();
    mDev.parse_param.prediction_width = ui->predictionWidth->value();
    mDev.parse_param.scan_time = ui->scanCycleTime->value();
    mDev.parse_param.video_overlap_cnt = ui->video_or_count->value();

    if(mDev.parse_param.video_overlap_cnt > mDev.parse_param.video_color_list.size())
    {
        QMessageBox::information(0,QString::fromUtf8("提示"),QString::fromUtf8("回波周期的颜色未设定！"),QString::fromUtf8("确认"));
        return;
    }

    emit signalNewParseSetting(mDev.toJson());

}

void zchxRadarUi::on_ok_interface_clicked()
{
    mDev.base.center.lat = ui->lat->value();
    mDev.base.center.lon = ui->lon->value();
    mDev.base.name = ui->name->text();
    if(mDev.channel_list.size() == 0) mDev.channel_list.append(zchxCommon::zchxRadarChannel());
    zchxCommon::zchxRadarChannel &channel = mDev.channel_list[0];
    channel.video.ip = ui->videoRecIPLlineEdit->text().trimmed();
    channel.video.port = ui->videoRecPortSpinBox->value();
    channel.report.ip = ui->controlIPLineEdit->text().trimmed();
    channel.report.port = ui->controlPortSpinBox->value();
    channel.heart.ip = ui->heartIPLlineEdit->text().trimmed();
    channel.heart.port = ui->heartPortSpinBox->value();
    emit signalModifiedBaseAndChannel(mDev.toJson());
}

void zchxRadarUi::on_lap_color_set_clicked()
{
    //根据叠加数设定颜色
    zchxVideoColorSettingWidget* w = new zchxVideoColorSettingWidget(ui->video_or_count->value(), mDev.parse_param.video_color_list);
    w->setWindowTitle(QString::fromUtf8("回波颜色设定"));
    w->setWindowFlags(Qt::CustomizeWindowHint|Qt::WindowCloseButtonHint);
    w->setAttribute(Qt::WA_DeleteOnClose);
    connect(w, &zchxVideoColorSettingWidget::signalSetFinished, this, [=](){
        w->close();
        mDev.parse_param.video_color_list = w->getColorList();
    });
    w->show();

}
