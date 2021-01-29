#include "zchxmainwindow.h"
#include "ui_zchxmainwindow.h"
#include "radardataoutputsettings.h"
#include <QDebug>
#include <QProcess>
#include "profiles.h"
#include "zchxlogindlg.h"
#include "qt/map_layer/zchxMapLayer.h"
#include "watchdogthread.h"
#include <QVBoxLayout>
#include "zchxradarbasesettingdlg.h"
#include "zchxradarparsesettingdlg.h"
#include "zchxradarchannelsettingdlg.h"
#include "zchxradarchannelreportctrldlg.h"
#include "zchxradarfilterareasettingdlg.h"
#include "zchxradarwidget.h"
#include <QSpacerItem>
#include "zchxradarui.h"


#define         LOG_LINE_COUNT          50
#define         PROPERTY_RADAR          "Radar"
#define         PROPERTY_AIS            "Ais"
#define         PROPERTY_TYPE           "TYPE"
#define         PROPERTY_ID             "ID"
#define         SEC_SERVER              "SERVER"
#define         KEY_HOST                "Host"
#define         KEY_PORT                "Port"

zchxMainWindow::zchxMainWindow(QWidget *parent) :
    QMainWindow(parent),
    mCurCfg(0),
    mDataReq(0),
    mDataChange(0),
    m_pEcdisWin(0),
    ui(new Ui::zchxMainWindow)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
//    ui->statusBar->setVisible(false);
    ui->ch1_btn->setFlat(false);
    ui->ch2_btn->setFlat(false);
    ui->ch1_btn->setChecked(false);
    ui->ch2_btn->setChecked(false);

    this->setWindowTitle(QString::fromUtf8("雷达监控系统客户端"));
    this->setWindowIcon(QIcon(":/image/app.png"));
    PROFILES_INSTANCE->setDefault(SEC_SERVER, KEY_HOST, "192.168.30.252");
    PROFILES_INSTANCE->setDefault(SEC_SERVER, KEY_PORT, 6666);

    //看门狗线程
    mapWatchDogThread *dog = new mapWatchDogThread(this);
    dog->start();

    //添加雷达显示
    ui->Ch1->setVisible(false);
    ui->Ch2->setVisible(false);
    QList<zchxRadarWidget*>  radar_w_list;
    radar_w_list.append(ui->Ch1);
    radar_w_list.append(ui->Ch2);
    checkSpacer();
    //添加菜单
    initMapSource(0);

    QTimer::singleShot(100, [=]()
    {
        QString host = PROFILES_INSTANCE->value(SEC_SERVER, KEY_HOST, "127.0.0.1").toString();
        int port = PROFILES_INSTANCE->value(SEC_SERVER, KEY_PORT, 6666).toInt();

        mDataReq = new zchxRequestWorker();
        connect(mDataReq, SIGNAL(signalSendErrorMsg(QString,int)), ui->statusBar, SLOT(showMessage(QString,int)));
        connect(mDataReq, SIGNAL(signalSendSvrCfg(QJsonObject)), this, SLOT(slotRecvNewCfg(QJsonObject)));
        connect(mDataReq, SIGNAL(signalRecheckHostPort(QString,int)), this, SLOT(slotRecheckHostAndPort(QString,int)));
        mDataReq->signalSetHostInfo(host, port);
        foreach (zchxRadarWidget *w, radar_w_list) {
            //雷达基础信息编辑
            connect(w->getRadarCtrl(), &zchxRadarUi::signalModifiedBaseAndChannel, [=](const QJsonValue& val){
                zchxCommon::zchxRadarDevice dev(val.toObject());
                qDebug()<<"new dev:"<<dev.toJson();
                emit mDataReq->signalUpdateCfg(zchxCommon::Msg_Update_BaseAndChannel, dev.toJson());
            });

            connect(w->getRadarCtrl(), &zchxRadarUi::signalNewParseSetting, [=](const QJsonValue& val){
                zchxCommon::zchxRadarDevice dev(val.toObject());
                qDebug()<<"new dev:"<<dev.toJson();
                emit mDataReq->signalUpdateCfg(zchxCommon::Msg_Update_ParseSetting, dev.toJson());
            });

            //雷达状态操作
            QList<zchxRadarChannelReportCtrlDlg*> reportDlgList;
            reportDlgList.append(w->getReportDlg());
            reportDlgList.append(w->getFanCtrlDlg());
            foreach (zchxRadarChannelReportCtrlDlg* dlg, reportDlgList) {
                connect(dlg, &zchxRadarChannelReportCtrlDlg::signalCtrlValChanged, [=](int type, QJsonValue val)
                {
                    zchxCommon::zchxRadarCtrlDef def;
                    def.radar_id = dlg->getRadarID();
                    def.ch_id = dlg->getChannelID();
                    def.type = type;
                    def.jsval = val;
                    qDebug()<<"send cmd:"<<def.toJson();
                    emit mDataReq->signalSetRadarCtrl(def.toJson());

                });
            }

            //屏蔽区域删除
            connect(w->getEcdis(), &qt::MainWindow::signalRemoveItem, [=](int type, int id){
                qDebug()<<"remove obj:"<<type<<id;
                if(type == ZCHX::Data::ELE_ISLAND_LINE)
                {
                    qDebug()<<"remove filter:"<<id;
                    zchxCommon::zchxFilterArea area;
                    area.id = id;
                    emit mDataReq->signalUpdateCfg(zchxCommon::Msg_Delete_FilterArea, area.toJson());
                }
            });

            connect(w->getEcdis(), SIGNAL(itfNewRadarFilterArea(std::vector<std::pair<double,double> >)), this, SLOT(slotNewRadarFilterArea(std::vector<std::pair<double,double> >)));

            //截屏
            connect(w->getEcdis(), &qt::MainWindow::itfSignalScreenShot, [=](){
                QDir curDir(QApplication::applicationDirPath());
                if(!curDir.exists("ScreenShot"))
                {
                    curDir.mkpath("ScreenShot");
                }
                QString fileName = QString("%1/ScreenShot/%2.png").arg(curDir.absolutePath()).arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz"));
                bool sts = w->getEcdis()->grab().save(fileName, "PNG");
                qDebug()<<"save img:"<<fileName<<" sts:"<<sts;
            });


        }


        mDataChange = new ZCHXRadarDataChange;
        connect(mDataChange, &ZCHXRadarDataChange::signalSendRadarVideo, [=](const QList<ZCHX::Data::ITF_RadarVideoImage>& list)
        {
            foreach (ZCHX::Data::ITF_RadarVideoImage img, list) {
                foreach (zchxRadarWidget *w, radar_w_list) {
                    if(img.radar_id.toInt() == w->getRadarId() && img.channel_id == w->getChannelID())
                    {
                        if(w->isVisible())
                        {
                            w->getEcdis()->itfSetRadarVideoData(QList<ZCHX::Data::ITF_RadarVideoImage>()<<img);
                        }
                        break;
                    }
                }
            }
        });
        connect(mDataChange, &ZCHXRadarDataChange::signalSendRadarPoint,[=](const QMap<int, QList<ZCHX::Data::ITF_RadarPoint> >& map)
        {
            foreach (int id, map.keys()) {
                qDebug()<<"recv radar:"<<id<<" point size:"<<map.value(id).size();
                foreach (zchxRadarWidget *w, radar_w_list) {
                    if(id == w->getRadarId())
                    {
                        if(w->isVisible())
                        {
                            w->getEcdis()->itfSetRadarPointData(id, map.value(id));
                        }
                        break;
                    }
                }
            }
        });

        connect(mDataChange, SIGNAL(signalUpdateRadarType(zchxCommon::zchxRadarTypeData)),
                this, SLOT(slotRecvRadarType(zchxCommon::zchxRadarTypeData)));
        connect(mDataChange, SIGNAL(signalUpdateRadarChannelRadius(int,int,double,double)),
                this, SLOT(slotUpdateRadarChannelRadius(int,int,double,double)));

        connect(mDataChange, &ZCHXRadarDataChange::signalSendNewPublishSetting, [=](const zchxCommon::zchxPublishSettingsList& list){
            mCurCfg->publish_list = list;
            mDataChange->appendPublishSetting(mCurCfg->server.ip, mCurCfg->publish_list);
        });

        connect(mDataChange, &ZCHXRadarDataChange::signalSendLimitDataList, [=](const QList<ZCHX::Data::ITF_IslandLine>& list)
        {
            foreach (zchxRadarWidget *w, radar_w_list) {
                if(w->isVisible()){
                    w->getEcdis()->itfSetIslandLineData(list);
                }
            }
        });

        connect(mDataChange, &ZCHXRadarDataChange::signalSendRadarNodeLog, [=](const QList<ZCHX::Data::ITF_RadarNodeLog>& list)
        {
            foreach (zchxRadarWidget *w, radar_w_list) {
                if(w->isVisible()){
                    w->getEcdis()->itfSetRadarNodeLogList(list);
                }
            }
        });


        connect(mDataChange, SIGNAL(signalUpdatePublishPortStatus(zchxCommon::zchxPublishSettingsList)),
                this, SLOT(slotUpdatePublishSettingSts(zchxCommon::zchxPublishSettingsList)));
        connect(mDataChange, SIGNAL(signalSendNewDevList(zchxCommon::zchxRadarDeviceList)),
                this, SLOT(updateAllRadarSettings(zchxCommon::zchxRadarDeviceList)));
        connect(mDataChange, SIGNAL(signalSendRadarReport(zchxCommon::zchxRadarReportList)),
                this, SLOT(slotRecvRadarReportStatusList(zchxCommon::zchxRadarReportList)));


        //添加图层控制
        QMenu* layerCtrlMenu = ui->menuBar->addMenu(QString::fromUtf8("图层显示"));
        QAction* radar_video = layerCtrlMenu->addAction(QString::fromUtf8("回波图层"));
        radar_video->setCheckable(true);
        connect(radar_video, &QAction::triggered, [=](bool sts){
            foreach (zchxRadarWidget *w, radar_w_list) {
                std::shared_ptr<qt::MapLayer> layer = w->getEcdis()->itfGetLayer(ZCHX::LAYER_RADARVIDEO);
                if(layer) layer->setVisible(sts);
            }
        });
        radar_video->setChecked(true);

        QAction* radar_target = layerCtrlMenu->addAction(QString::fromUtf8("雷达目标图层"));
        radar_target->setCheckable(true);
        connect(radar_target, &QAction::triggered, [=](bool sts){
            foreach (zchxRadarWidget *w, radar_w_list) {
                std::shared_ptr<qt::MapLayer> layer = w->getEcdis()->itfGetLayer(ZCHX::LAYER_RADAR_CURRENT);
                if(layer) layer->setVisible(sts);
            }
        });
        radar_target->setChecked(true);

        QAction* radar_history = layerCtrlMenu->addAction(QString::fromUtf8("雷达历史轨迹图层"));
        radar_history->setCheckable(true);
        connect(radar_history, &QAction::triggered, [=](bool sts){
            foreach (zchxRadarWidget *w, radar_w_list) {
                w->getEcdis()->itfSetRadarHistoryTrackDisplay(sts);
            }
        });
        radar_history->setChecked(true);

        QAction* radar_glow = layerCtrlMenu->addAction(QString::fromUtf8("雷达余晖图层"));
        radar_glow->setCheckable(true);
        connect(radar_glow, &QAction::triggered, [=](bool sts){
            foreach (zchxRadarWidget *w, radar_w_list) {
                w->getEcdis()->itfSetRadarGlowDisplay(sts);
            }
        });
        radar_glow->setChecked(true);


        QAction* filter_act = layerCtrlMenu->addAction(QString::fromUtf8("屏蔽区域"));
        filter_act->setCheckable(true);
        connect(filter_act, &QAction::triggered, [=](bool sts){
            foreach (zchxRadarWidget *w, radar_w_list) {
                std::shared_ptr<qt::MapLayer> layer = w->getEcdis()->itfGetLayer(ZCHX::LAYER_ISLAND);
                if(layer) layer->setVisible(sts);
            }
        });
        filter_act->setChecked(true);


        QAction* log_act = layerCtrlMenu->addAction(QString::fromUtf8("雷达删除日志"));
        log_act->setCheckable(true);
        connect(log_act, &QAction::triggered, [=](bool sts){
            foreach (zchxRadarWidget *w, radar_w_list) {
                std::shared_ptr<qt::MapLayer> layer = w->getEcdis()->itfGetLayer(ZCHX::LAYER_RADAR_LOG);
                if(layer) layer->setVisible(sts);
            }
        });
        log_act->setChecked(true);
    });

}


void zchxMainWindow::slotRecheckHostAndPort(const QString &host, int port)
{
    qDebug()<<"recheck host and port:"<<host<<port;
    zchxLoginDlg* dlg = new zchxLoginDlg(host, port, this);
    dlg->setModal(false);
    dlg->setWindowFlags(dlg->windowFlags() | Qt::Tool);
    connect(dlg, &zchxLoginDlg::signalNewHostAndPort, [=](QString new_ip, int new_port)
    {
        PROFILES_INSTANCE->setValue(SEC_SERVER, KEY_HOST, new_ip);
        PROFILES_INSTANCE->setValue(SEC_SERVER, KEY_PORT, new_port);
        restartMe();
    });

    connect(dlg, &zchxLoginDlg::rejected, [=]()
    {
        emit mDataReq->signalSetHostInfo(host, port);
    });
    dlg->exec();
}

void zchxMainWindow::slotSetSvrHost()
{
    QString host = PROFILES_INSTANCE->value(SEC_SERVER, KEY_HOST).toString();
    int port = PROFILES_INSTANCE->value(SEC_SERVER, KEY_PORT).toInt();
    slotRecheckHostAndPort(host, port);
}

void zchxMainWindow::slotStartEcdis()
{
    m_pEcdisWin = new qt::MainWindow;
    ui->centralWidget->layout()->addWidget(m_pEcdisWin);

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

    initMapSource(0);
    emit signalMapLoadFinished();
}

void zchxMainWindow::initMapSource(int source)
{
    //添加菜单控制
    mMapSouceMenu = ui->menuBar->addMenu(QStringLiteral("地图数据源指定"));
    QStringList mapSourceTitles;
    mapSourceTitles.append(QString::fromUtf8("本地地图"));
    mapSourceTitles.append(QString::fromUtf8("谷歌在线地图"));
    for(int i=0; i<=2; i+=2)
    {
        QAction* act = mMapSouceMenu->addAction(mapSourceTitles.takeFirst());
        act->setData(i);
        connect(act, SIGNAL(triggered(bool)), this, SLOT(slotSetMapSource()));
        act->setCheckable(true);
        qDebug()<<"menu data:"<<act->data().toInt()<<source;
        if(act->data().toInt() == source)
        {
            act->setChecked(true);
        }
    }
}

void zchxMainWindow::slotSetMapSource()
{
    QAction *act = qobject_cast<QAction*> (sender());
    if(!act) return;
    QList<QAction*> actList = mMapSouceMenu->actions();
    foreach (QAction* temp, actList) {
        if(temp == act)
        {
            temp->setChecked(true);
        }
        else
        {
            temp->setChecked(false);
        }
    }

    int mode = act->data().toInt();
    ui->Ch1->setMapSource(ZCHX::TILE_SOURCE(mode));
    ui->Ch2->setMapSource(ZCHX::TILE_SOURCE(mode));
}

void zchxMainWindow::slotRemoveRadar(int id)
{
    QList<QAction*> actList = mRadarSetAct->menu()->actions();
    foreach (QAction* act, actList) {
        if(act && act->data().canConvert<zchxCommon::zchxRadarDevice>())
        {
            if(act->data().value<zchxCommon::zchxRadarDevice>().base.id == id)
            {
                mRadarSetAct->menu()->removeAction(act);
                break;
            }
        }
    }
}

//这里就是服务端的整体配置
void zchxMainWindow::slotRecvNewCfg(const QJsonObject& cfg)
{
    ui->statusBar->showMessage(QString::fromUtf8("收到服务端新的配置信息"));
    if(!mCurCfg) delete mCurCfg;
    mCurCfg = new zchxCommon::zchxRadarServerCfg(cfg);
    //1)获取当前的数据推送设定，更新数据推送接口
    qDebug()<<"recv publish setting size:"<<mCurCfg->publish_list.size();
    QString host = PROFILES_INSTANCE->value(SEC_SERVER, KEY_HOST).toString();
    if(mDataChange) mDataChange->appendPublishSetting(host, mCurCfg->publish_list);
    slotUpdatePublishSettingSts(mCurCfg->publish_list);


    //2)更新当前的雷达设定
    qDebug()<<"recv radar device size:"<<mCurCfg->device_list.size();
    updateAllRadarSettings(mCurCfg->device_list);
}

void zchxMainWindow::updateAllRadarSettings(const zchxCommon::zchxRadarDeviceList &list)
{
    if(list.size() > 0) ui->Ch1->setRadarDev(list.first());
    if(list.size() > 1) ui->Ch2->setRadarDev(list.last());
}

void zchxMainWindow::closeEvent(QCloseEvent *e)
{
    qDebug()<<"start close window...";
    QMainWindow::closeEvent(e);

}

zchxMainWindow::~zchxMainWindow()
{
    if(mDataReq) delete mDataReq;
    if(mDataChange) delete mDataChange;
    delete ui;
    qDebug()<<"main window ~~~~~";

}

int zchxMainWindow::showMessageBox(const QString& content, bool cancel)
{
    QMessageBox::StandardButtons btns = QMessageBox::Ok;
    if(cancel) btns |= QMessageBox::Cancel;
    QMessageBox box(QMessageBox::Information, QString::fromUtf8("提示"), content, btns);
    box.setButtonText(QMessageBox::Ok, QString::fromUtf8("确认"));
    if(cancel)  box.setButtonText(QMessageBox::Cancel, QString::fromUtf8("取消"));
    return box.exec();
}

//AIS菜单按下
void zchxMainWindow::slotNewAis()
{
//    ais_index++;
//    Utils::Profiles::instance()->setValue("Ais","AIS_Num",ais_index);
//    AIS_Setting *mAisSetting = new AIS_Setting(ais_index);
//    ui->tabWidget->insertTab(ui->tabWidget->count(), mAisSetting,"AIS-"+QString::number(ais_index));
//    //重新生成AIS解析对象
//    connect(mAisSetting,SIGNAL(newAisClassSignal()),this,SLOT(newAisClassSlot()));
}

void zchxMainWindow::slotEditRadar()
{
    qDebug()<<"radar edit clicked now.....";
    if(mAddRadarAct) mAddRadarAct->setVisible(true);
    if(m_pEcdisWin) m_pEcdisWin->setVisible(false);
}

void zchxMainWindow::slotExitEdit()
{
    if(m_pEcdisWin) m_pEcdisWin->setVisible(true);
    if(mAddRadarAct) mAddRadarAct->setVisible(false);
}


void zchxMainWindow::addRadarDev(zchxCommon::zchxRadarDevice* dev)
{
    qDebug()<<dev->toJson();
    //添加显示雷达设备名的主菜单项
    QAction* sub_radar_act = mRadarSetAct->menu()->addMenu(new QMenu(dev->base.name));
    if(!sub_radar_act) return;
    sub_radar_act->setData(QVariant::fromValue(*dev));   //将雷达设备的值绑定在菜单项上

    //添加对应的名字编辑菜单
    QAction* nameAct = sub_radar_act->menu()->addAction(QString::fromUtf8("配置基础信息"));
    nameAct->setData(QVariant::fromValue(dev->base));
    connect(nameAct, &QAction::triggered, [=](){
        zchxRadarBaseSettingDlg* dlg = new zchxRadarBaseSettingDlg(dev->base);
        dlg->setAttribute(Qt::WA_DeleteOnClose);
        dlg->setModal(false);
        connect(dlg, &zchxRadarBaseSettingDlg::signalnewSetting, [=](const QJsonValue& val){
            dev->base = zchxCommon::zchxRadarDeviceBaseSetting(val.toObject());
            emit mDataReq->signalUpdateCfg(zchxCommon::Msg_Update_RadarBaseSetting, dev->toJson());
        });
        dlg->show();
        dlg->raise();
    });
    //添加对应的参数编辑设定
    QAction* parseAct = sub_radar_act->menu()->addAction(QString::fromUtf8("配置回波解析参数"));
    parseAct->setData(QVariant::fromValue(dev->parse_param));
    connect(parseAct, &QAction::triggered, [=](){
        zchxRadarParseSettingDlg* dlg = new zchxRadarParseSettingDlg(dev->parse_param);
        dlg->setAttribute(Qt::WA_DeleteOnClose);
        dlg->setModal(false);
        connect(dlg, &zchxRadarParseSettingDlg::signalnewSetting, [=](const QJsonValue& val){
            dev->parse_param = zchxCommon::zchxVideoParse(val.toObject());
            emit mDataReq->signalUpdateCfg(zchxCommon::Msg_Update_ParseSetting, dev->toJson());
        });
        dlg->show();
        dlg->raise();

    });
    //添加通道子菜单
    QAction* channel_act = sub_radar_act->menu()->addMenu(new QMenu(QString::fromUtf8("配置通道")));
    //新的通道手动添加
    QAction* channel_add_act = channel_act->menu()->addAction(QString::fromUtf8("添加通道"));
    connect(channel_add_act, &QAction::triggered, [=](){
        zchxRadarChannelSettingDlg* dlg = new zchxRadarChannelSettingDlg(zchxCommon::zchxRadarChannel());
        dlg->setAttribute(Qt::WA_DeleteOnClose);
        dlg->setModal(false);
        connect(dlg, &zchxRadarChannelSettingDlg::signalNewSetting, [=](const QJsonValue& val){
            dev->channel_list.append(zchxCommon::zchxRadarChannel(val.toObject()));
            emit mDataReq->signalUpdateCfg(zchxCommon::Msg_Update_ChannelSetting, dev->toJson());
        });
        dlg->show();
        dlg->raise();

    });
    //添加已经存在的通道到菜单
    foreach (zchxCommon::zchxRadarChannel channel, dev->channel_list) {
        //1)添加通道名
        QAction* target_act = channel_act->menu()->addMenu(new QMenu(QString::fromUtf8("通道%1").arg(channel.id)));
        target_act->setData(QVariant::fromValue(channel));
        target_act->menu()->addAction(QString::fromUtf8("通道数据接收设定"), [=]()
        {
            zchxRadarChannelSettingDlg* dlg = new zchxRadarChannelSettingDlg(channel);
            dlg->setAttribute(Qt::WA_DeleteOnClose);
            dlg->setModal(false);
            connect(dlg, &zchxRadarChannelSettingDlg::signalNewSetting, [=](const QJsonValue& val){
                zchxCommon::zchxRadarChannel new_channel(val.toObject());
                for(int i=0; i<dev->channel_list.size(); i++)
                {
                    if(new_channel.id == dev->channel_list[i].id)
                    {
                        dev->channel_list.replace(i, new_channel);
                        break;
                    }
                }
                emit mDataReq->signalUpdateCfg(zchxCommon::Msg_Update_ChannelSetting, dev->toJson());
            });
            dlg->show();
            dlg->raise();
        });

        target_act->menu()->addAction(QString::fromUtf8("删除通道"), [=]()
        {            
            int del_id = target_act->data().value<zchxCommon::zchxRadarChannel>().id;
            qDebug()<<"before delete channel size:"<<dev->channel_list.size()<<" to be deleted id:"<<del_id;
            for(int i=0; i<dev->channel_list.size(); i++)
            {
                if(del_id == dev->channel_list[i].id)
                {
                    qDebug()<<"channel has been deleted at index:"<<del_id<<" index:"<<i;
                    dev->channel_list.removeAt(i);
                    break;
                }
            }
            qDebug()<<"after delete channel size:"<<dev->channel_list.size();
            emit mDataReq->signalUpdateCfg(zchxCommon::Msg_Update_ChannelSetting, dev->toJson());
        });
    }

    //添加删除项
    QAction* delAct = sub_radar_act->menu()->addAction(QString::fromUtf8("删除"));
    delAct->setData(dev->base.id);
    connect(delAct, &QAction::triggered, [=](){
        emit mDataReq->signalUpdateCfg(zchxCommon::Msg_Delete_Radar, QJsonValue(dev->base.id));
    });


}

void zchxMainWindow::slotCfgChanged()
{
    ui->centralWidget->setEnabled(false);
}

void zchxMainWindow::restartMe()
{
    QString program = QApplication::applicationFilePath();
    QStringList arguments = QApplication::arguments();
    QProcess::startDetached(program, arguments, QApplication::applicationDirPath());
    QApplication::exit();
}

void zchxMainWindow::slotRecvPortStartStatus(const zchxCommon::zchxPortStatusList& list)
{
    if(!mStatusMenu) return;
    mStatusMenu->clear();
    foreach (zchxCommon::zchxPortStatus sts , list) {
        mStatusMenu->addAction(new QAction(sts.toString(), this));
    }
}

void zchxMainWindow::slotSetDataPublish()
{
    if(!mCurCfg) return;
    RadarDataOutputSettings* dlg = new RadarDataOutputSettings(mCurCfg);
    connect(dlg, &RadarDataOutputSettings::signalResetPublishSettings, [=]()
    {
        QJsonValue val = mCurCfg->publish_list.toJson();
        emit mDataReq->signalUpdateCfg(zchxCommon::Msg_Update_PublishSetting, val);
    });
    dlg->show();
}


void zchxMainWindow::slotUpdatePublishSettingSts(const zchxCommon::zchxPublishSettingsList &list)
{
    QMap<int, zchxCommon::zchxPortStatus> map;
    foreach (zchxCommon::zchxPublishSetting set, list) {
        zchxCommon::zchxPortStatus &port_sts = map[set.port];
        port_sts.port = set.port;
        port_sts.sts = set.status;
        if(port_sts.topic.size() > 0) port_sts.topic.append(",");
        port_sts.topic.append(set.topic);
        qDebug()<<"publish port status:"<<port_sts.toString();
    }
    zchxCommon::zchxPortStatusList result;
    result.append(map.values());
    slotRecvPortStartStatus(result);
}

void zchxMainWindow::slotRecvRadarVideoImages(const QList<ZCHX::Data::ITF_RadarVideoImage> &list)
{
    if(!m_pEcdisWin) return;
    m_pEcdisWin->itfSetRadarVideoData(list);
}

void zchxMainWindow::slotRecvRadarReportStatusList(const zchxCommon::zchxRadarReportList &list)
{
#if 0
    foreach (zchxCommon::zchxRadarChannelReport report, list) {
        QString key = QString("%1_%2").arg(report.mRadarID).arg(report.mChannelID);
        QSharedPointer<zchxRadarChannelReportCtrlDlg> dlg = mRadarReportMapDlg[key];
        if(!dlg)
        {
            dlg = QSharedPointer<zchxRadarChannelReportCtrlDlg>(new zchxRadarChannelReportCtrlDlg);
            dlg->setModal(false);
            dlg->setWindowTitle(QString::fromUtf8("雷达状态控制"));            
            dlg->setWindowFlags(Qt::CustomizeWindowHint|Qt::WindowCloseButtonHint);
            mRadarReportMapDlg[key] = dlg;
            connect(dlg.data(), &zchxRadarChannelReportCtrlDlg::signalCtrlValChanged, [=](int type, QJsonValue val)
            {
                zchxCommon::zchxRadarCtrlDef def;
                def.radar_id = dlg->getRadarID();
                def.ch_id = dlg->getChannelID();
                def.type = type;
                def.jsval = val;
                emit mDataReq->signalSetRadarCtrl(def.toJson());

            });
        }
        dlg->setReportInfo(report.reportList, report.mRadarID, report.mChannelID);
    }

#else
    foreach (zchxCommon::zchxRadarChannelReport report, list) {
        QString key = QString("%1_%2").arg(report.mRadarID).arg(report.mChannelID);
        zchxRadarChannelReportCtrlDlg* dlg = mRadarReportDlgMap[key];
        if(!dlg)
        {
            if(ui->Ch1->getRadarId() == report.mRadarID && ui->Ch1->getChannelID() == report.mChannelID)
            {
                dlg = ui->Ch1->getReportDlg();

            } else if(ui->Ch2->getRadarId() == report.mRadarID && ui->Ch2->getChannelID() == report.mChannelID)
            {
                dlg = ui->Ch2->getReportDlg();
            }
            if(dlg) mRadarReportDlgMap[key] = dlg;
        }
        if(dlg)
        {
            dlg->setReportInfo(report.reportList, report.mRadarID, report.mChannelID);
        }
    }
#endif
}

void zchxMainWindow::slotRecvRadarType(const zchxCommon::zchxRadarTypeData &data)
{
    QString key = QString("%1_%2").arg(data.radar_id).arg(data.channel_id);
    zchxRadarChannelReportCtrlDlg* dlg = mRadarReportDlgMap[key];
    if(dlg)
    {
        dlg->setRadarType(data.type);
    }

}

void zchxMainWindow::slotUpdateRadarChannelRadius(int radar, int channel, double radius, double factor)
{
    QString key = QString("%1_%2").arg(radar).arg(channel);
    zchxRadarChannelReportCtrlDlg* dlg = mRadarReportDlgMap[key];
    if(dlg)
    {
        dlg->updateRangefactor(radius, factor);
    }
}

void zchxMainWindow::slotNewRadarFilterArea(const std::vector<std::pair<double, double> > &path)
{
    zchxRadarFilterAreaSettingDlg* dlg = new zchxRadarFilterAreaSettingDlg(this);
    dlg->setWindowFlags(dlg->windowFlags() | Qt::Tool);
    dlg->setModal(false);
    zchxCommon::zchxFilterArea data;
    data.area = zchxRadarFilterAreaSettingDlg::getAreaFromPath(path);
    data.time = QDateTime::currentMSecsSinceEpoch();
    dlg->setCurFilterData(data);

    connect(dlg, &zchxRadarFilterAreaSettingDlg::signalNewSetting, [=](const QJsonValue& val){
        emit mDataReq->signalUpdateCfg(zchxCommon::Msg_Edit_FilterArea, val);
    });
    dlg->exec();
}


void zchxMainWindow::on_ch1_btn_clicked()
{
//    ui->ch1_btn->setFlat(!ui->ch1_btn->isFlat());
    ui->Ch1->setChannelNum(1);
    ui->Ch1->setVisible(ui->ch1_btn->isChecked());
    if(!ui->ch1_btn->isChecked())
    {
        ui->ch1_btn->setFlat(false);
    }
    checkSpacer();
}

void zchxMainWindow::on_ch2_btn_clicked()
{
//    ui->ch2_btn->setFlat(!ui->ch2_btn->isFlat());
    ui->Ch2->setChannelNum(2);
    ui->Ch2->setVisible(ui->ch2_btn->isChecked());
    if(!ui->ch2_btn->isChecked())
    {
        ui->ch2_btn->setFlat(false);
    }

    checkSpacer();
}

void zchxMainWindow::checkSpacer()
{
    if(ui->ch1_btn->isChecked() || ui->ch2_btn->isChecked())
    {
        for(int i=0; i<ui->verticalLayout_2->count(); i++)
        {
            if(ui->verticalLayout_2->itemAt(i)->spacerItem())
            {
                ui->verticalLayout_2->removeItem(ui->verticalLayout_2->itemAt(i));
                break;
            }
        }
    } else
    {
        QSpacerItem *verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
        ui->verticalLayout_2->addItem(verticalSpacer);
    }
}
