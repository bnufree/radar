#ifndef ZCHXRADARWIDGET_H
#define ZCHXRADARWIDGET_H

#include <QWidget>
#include "qt/mainwindow.h"
#include "zchxdatadef.h"

namespace Ui {
class zchxRadarWidget;
}
class zchxRadarUi;
class zchxRadarChannelReportCtrlDlg;
class zchxRadarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit zchxRadarWidget(QWidget *parent = 0);
    ~zchxRadarWidget();
    void     setChannelNum(int id);
    void     setRadarDev(const zchxCommon::zchxRadarDevice& dev);
    qt::MainWindow*  getEcdis() const {return m_pEcdisWin;}
    zchxRadarUi*     getRadarCtrl() const;
    int      getRadarId() const {return mDev.base.id;}
    int      getChannelID() const {if(mDev.channel_list.size() > 0) return mDev.channel_list.first().id; return -1;}
    zchxRadarChannelReportCtrlDlg*  getReportDlg();
    zchxRadarChannelReportCtrlDlg*  getFanCtrlDlg();



public slots:
    void     setMapSource(int mode);

private:
    Ui::zchxRadarWidget *ui;
    qt::MainWindow      *m_pEcdisWin;
    zchxCommon::zchxRadarDevice mDev;
};

#endif // ZCHXRADARWIDGET_H
