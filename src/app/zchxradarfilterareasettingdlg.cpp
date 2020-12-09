#include "zchxradarfilterareasettingdlg.h"
#include "ui_zchxradarfilterareasettingdlg.h"
#include <QDateTime>

zchxRadarFilterAreaSettingDlg::zchxRadarFilterAreaSettingDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::zchxRadarFilterAreaSettingDlg)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    this->setWindowFlags(Qt::CustomizeWindowHint|Qt::WindowCloseButtonHint);
    ui->filter_type_cbx->addItem(QString::fromUtf8("保留区域外目标"), 0);
    ui->filter_type_cbx->addItem(QString::fromUtf8("保留区域内目标"), 1);
    setWindowTitle(QString::fromUtf8("添加过滤区域"));
    mCurData.id = 0;
}

zchxRadarFilterAreaSettingDlg::~zchxRadarFilterAreaSettingDlg()
{
    delete ui;
}

void zchxRadarFilterAreaSettingDlg::setCurFilterData(const zchxCommon::zchxFilterArea &data)
{
    mCurData = data;
    if(mCurData.id > 0)
    {
        setWindowTitle(QString::fromUtf8("编辑过滤区域"));
    } else
    {
        setWindowTitle(QString::fromUtf8("添加过滤区域"));
    }
    ui->filter_type_cbx->setCurrentIndex(mCurData.type);
    ui->name->setText(mCurData.name);
    ui->create_time->setText(QDateTime::fromMSecsSinceEpoch(mCurData.time).toString("yyyy-MM-dd hh:mm:ss"));
}

void zchxRadarFilterAreaSettingDlg::recvNewFilterArea(const std::vector<std::pair<double, double> > &path)
{

}

void zchxRadarFilterAreaSettingDlg::on_filter_type_cbx_currentIndexChanged(int index)
{
    mCurData.type = ui->filter_type_cbx->currentData().toInt();
}

void zchxRadarFilterAreaSettingDlg::on_edit_clicked()
{

}

void zchxRadarFilterAreaSettingDlg::on_look_clicked()
{

}

void zchxRadarFilterAreaSettingDlg::on_del_clicked()
{

}

void zchxRadarFilterAreaSettingDlg::on_ok_clicked()
{
    mCurData.name = ui->name->text().trimmed();
    mCurData.type = ui->filter_type_cbx->currentData().toInt();
    emit signalNewSetting(mCurData.toJson());
    close();
}

zchxCommon::zchxArea    zchxRadarFilterAreaSettingDlg::getAreaFromPath(const std::vector<std::pair<double, double> > &path)
{
    zchxCommon::zchxArea data;
    for(int i=0; i<path.size(); i++)
    {
        std::pair<double, double> ll = path.at(i);
        data.append(zchxCommon::zchxLatlon(ll.first, ll.second));
    }

    return data;
}
