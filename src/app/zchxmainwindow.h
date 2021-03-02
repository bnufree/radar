#ifndef ZCHXMAINWINDOW_H
#define ZCHXMAINWINDOW_H

#include "qt/mainwindow.h"
#include <QMainWindow>
#include "zchxdatadef.h"
#include "network/zchxrequestworker.h"
#include "network/zchxradardatachange.h"


class QLabel;
namespace Ui {
class zchxMainWindow;
}

class QProcess;
class zchxRadarChannelReportCtrlDlg;

//一个雷达设备对应的菜单项
struct RadarChannelMenuCollector{
    QAction*    mMainAct;
    QAction*    mEditAct;
    QAction*    mDelAct;

    RadarChannelMenuCollector() {mMainAct = 0; mEditAct = 0; mDelAct = 0;}
    ~RadarChannelMenuCollector()
    {
        if(mMainAct) delete mMainAct;
        if(mEditAct) delete mEditAct;
        if(mDelAct) delete mDelAct;
    }
};

struct RadarDevMenuCollector{
    QAction*    mMainAct;
    QAction*    mEditNameAct;
    QAction*    mAddChannelAct;
    QAction*    mEditParseAct;
    QAction*    mDelAct;
    QMenu*      mChannelMenu;
    QMap<int, QSharedPointer<RadarChannelMenuCollector>>        mChannelEditors;

    RadarDevMenuCollector() {mMainAct = 0; mEditNameAct = 0; mEditParseAct = 0; mAddChannelAct = 0;mDelAct = 0;}
    ~RadarDevMenuCollector()
    {
        if(mMainAct) delete mMainAct;
        if(mEditNameAct) delete mEditNameAct;
        if(mAddChannelAct) delete mAddChannelAct;
        if(mEditParseAct) delete mEditParseAct;
        if(mDelAct) delete mDelAct;
        mChannelEditors.clear();
    }
};

class zchxMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit zchxMainWindow(QWidget *parent = 0);
    ~zchxMainWindow();


signals:
    void signalCfgChanged();
    void signalMapLoadFinished();
protected:
    void closeEvent(QCloseEvent *);
private slots:
    void slotRecvNewCfg(const QJsonObject& cfg);
    void addRadarDev(zchxCommon::zchxRadarDevice* dev);
    void slotCfgChanged();
    void slotRecheckHostAndPort(const QString& host, int port);
    void slotStartEcdis();
    void slotRemoveRadar(int id);

public slots:
    void slotNewAis();//AIS菜单按下
    void slotEditRadar();
    void slotRecvPortStartStatus(const zchxCommon::zchxPortStatusList& list);
    void slotUpdatePublishSettingSts(const zchxCommon::zchxPublishSettingsList& list);
    void slotRecvRadarReportStatusList(const zchxCommon::zchxRadarReportList& list);
    void slotRecvRadarType(const zchxCommon::zchxRadarTypeData& data);
    void slotUpdateRadarChannelRadius(int radar, int channel, double radius, double factor);
    void slotSetSvrHost();
    void slotExitEdit();
    void slotNewRadarFilterArea(const std::vector<std::pair<double,double> >& path);

private slots:
    void slotRecvRadarVideoImages(const QList<ZCHX::Data::ITF_RadarVideoImage>& list);
    void restartMe();
    void slotSetDataPublish();
    void initMapSource(int source);
    void slotSetMapSource();
    int showMessageBox(const QString& content, bool cancel);
    void checkSpacer();
    void stopRecv();
private slots:
    void updateAllRadarSettings(const zchxCommon::zchxRadarDeviceList& list);

    void on_ch1_btn_clicked();

    void on_ch2_btn_clicked();

private:
    Ui::zchxMainWindow  *ui;
    QMenu*              mStatusMenu = 0;
    QMenu*              mMapSouceMenu = 0;
    QMenu*              mSvrCfgMenu = 0;
    zchxCommon::zchxRadarServerCfg* mCurCfg = 0;
    ZCHXRadarDataChange     *mDataChange = 0;
    zchxRequestWorker       *mDataReq = 0;
    qt::MainWindow         *m_pEcdisWin = 0;
    QAction*                mAddRadarAct = 0;
    QAction*                mRadarSetAct = 0;
    QAction*                mDataPubSetAct = 0;
    zchxCommon::zchxRadarDeviceList         mRadarList;
    QMap<QString, zchxRadarChannelReportCtrlDlg*>              mRadarReportDlgMap;
};

#endif // ZCHXMAINWINDOW_H
