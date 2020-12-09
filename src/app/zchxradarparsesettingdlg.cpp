#include "zchxradarparsesettingdlg.h"
#include "ui_zchxradarparsesettingdlg.h"

zchxRadarParseSettingDlg::zchxRadarParseSettingDlg(const zchxCommon::zchxVideoParse& parse, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::zchxRadarParseSettingDlg),
    mParse(parse)
{
    ui->setupUi(this);    
    this->setWindowFlags(Qt::CustomizeWindowHint|Qt::WindowCloseButtonHint);
    setAttribute(Qt::WA_DeleteOnClose);
    //解析参数初始化
    ui->target_length_filter_chk->setChecked(mParse.lenth.used);
    ui->target_length_min->setValue(mParse.lenth.min);
    ui->target_length_max->setValue(mParse.lenth.max);
    ui->target_area_filter_chk->setChecked(mParse.area.used);
    ui->target_area_min->setValue(mParse.area.min);
    ui->target_area_max->setValue(mParse.area.max);
    ui->video_amp_chk->setChecked(mParse.amp.used);
    ui->video_amplitude_min->setValue(mParse.amp.min);
    ui->video_amplitude_max->setValue(mParse.amp.max);
    ui->clear_track_time->setValue(mParse.clear_target_time);
    ui->max_history->setValue(mParse.max_history_num);
    ui->video_or_count->setValue(mParse.video_overlap_cnt);
    ui->predictionWidth->setValue(mParse.prediction_width);
    ui->targetMaxSpeedSpinBox->setValue(mParse.max_target_speed);
    ui->scanCycleTime->setValue(mParse.scan_time);
    ui->target_confirm_spinBox->setValue(mParse.confirm_target_cnt);
    ui->move_target_min_speed_spinBox->setValue(mParse.output_target_min_speed);
}

zchxRadarParseSettingDlg::~zchxRadarParseSettingDlg()
{
    delete ui;
}

void zchxRadarParseSettingDlg::on_pushButton_clicked()
{
    //保存目标解析参数
    mParse.lenth.used = ui->target_length_filter_chk->isChecked();
    mParse.lenth.min = ui->target_length_min->value();
    mParse.lenth.max = ui->target_length_max->value();
    mParse.amp.used = ui->video_amp_chk->isChecked();
    mParse.amp.min = ui->video_amplitude_min->value();
    mParse.amp.max = ui->video_amplitude_max->value();
    mParse.area.used = ui->target_area_filter_chk->isChecked();
    mParse.area.min = ui->target_area_min->value();
    mParse.area.max = ui->target_area_max->value();

    mParse.clear_target_time = ui->clear_track_time->value();
    mParse.confirm_target_cnt = ui->target_confirm_spinBox->value();
    mParse.max_history_num = ui->max_history->value();
    mParse.max_target_speed = ui->targetMaxSpeedSpinBox->value();
    mParse.output_target_min_speed = ui->move_target_min_speed_spinBox->value();
    mParse.prediction_width = ui->predictionWidth->value();
    mParse.scan_time = ui->scanCycleTime->value();
    mParse.video_overlap_cnt = ui->video_or_count->value();

    emit signalnewSetting(mParse.toJson());
    close();
}
