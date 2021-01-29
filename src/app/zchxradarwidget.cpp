#include "zchxradarwidget.h"
#include "ui_zchxradarwidget.h"
#include "qt/map_layer/zchxMapLayer.h"

zchxRadarWidget::zchxRadarWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::zchxRadarWidget)
{
    ui->setupUi(this);
    ui->gridLayout->addWidget(m_pEcdisWin = new qt::MainWindow(this));
    ui->ctrl_frame->setFixedWidth(ui->ctrl_widget->width());
    std::shared_ptr<qt::MapLayer> islandline(new qt::MapLayer(ZCHX::LAYER_ISLAND, ZCHX::TR_LAYER_ISLAND, true));
    m_pEcdisWin->itfAddLayer(islandline);

    //ais
    std::shared_ptr<qt::MapLayer> pAisLayer(new qt::MapLayer(ZCHX::LAYER_AIS,ZCHX::TR_LAYER_AIS, true));
    m_pEcdisWin->itfAddLayer(pAisLayer);
    //雷达
    std::shared_ptr<qt::MapLayer> pRadarTrackLayer(new qt::MapLayer(ZCHX::LAYER_RADAR_TRACK,ZCHX::TR_LAYER_RADAR_TRACK,true));
    m_pEcdisWin->itfAddLayer(pRadarTrackLayer);
    //回波
    std::shared_ptr<qt::MapLayer> pRadarVideo(new qt::MapLayer(ZCHX::LAYER_RADARVIDEO,ZCHX::TR_LAYER_RADARVIDEO, true));
    m_pEcdisWin->itfAddLayer(pRadarVideo);

    //
    m_pEcdisWin->setCtrlFrameVisible(false);
    m_pEcdisWin->itfSetRadarLabelVisible(true);
    connect(m_pEcdisWin, SIGNAL(signalSendCurPosAsRadarCenter(double,double)), ui->ctrl_widget, SLOT(setRadarCenter(double,double)));
}

zchxRadarWidget::~zchxRadarWidget()
{
    delete ui;
}

void zchxRadarWidget::setChannelNum(int id)
{
    ui->ctrl_widget->setChannelNum(id);
}

void zchxRadarWidget::setRadarDev(const zchxCommon::zchxRadarDevice &dev)
{
    mDev = dev;
    ui->ctrl_widget->setRadarDev(dev);
}

void zchxRadarWidget::setMapSource(int mode)
{
    if(m_pEcdisWin) m_pEcdisWin->setEcdisMapSource(ZCHX::TILE_SOURCE(mode));
}

zchxRadarUi* zchxRadarWidget::getRadarCtrl() const
{
    return ui->ctrl_widget;
}

zchxRadarChannelReportCtrlDlg* zchxRadarWidget::getReportDlg()
{
    return getRadarCtrl()->getReportDlg();
}

zchxRadarChannelReportCtrlDlg* zchxRadarWidget::getFanCtrlDlg()
{
    return getRadarCtrl()->getFanDlg();
}
