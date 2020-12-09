#ifndef ZCHXRADARUI_H
#define ZCHXRADARUI_H

#include <QWidget>
#include "zchxdatadef.h"

namespace Ui {
class zchxRadarUi;
}
class zchxRadarChannelReportCtrlDlg;

class zchxRadarUi : public QWidget
{
    Q_OBJECT

public:
    explicit zchxRadarUi(QWidget *parent = 0);
    ~zchxRadarUi();
    void  setChannelNum(int id);
    void  setRadarDev(const zchxCommon::zchxRadarDevice &dev);
    zchxRadarChannelReportCtrlDlg*  getReportDlg() const {return mReportDlg;}
    zchxRadarChannelReportCtrlDlg*  getFanDlg() const {return mFanCtrlDlg;}

public slots:
    void  setRadarCenter(double lat, double lon);
signals:
    void  signalModifiedBaseAndChannel(const QJsonValue& val);
    void  signalNewParseSetting(const QJsonValue& val);

private slots:
    void on_ok_parse_clicked();

    void on_ok_interface_clicked();

private:
    Ui::zchxRadarUi *ui;
    zchxCommon::zchxRadarDevice mDev;
    zchxRadarChannelReportCtrlDlg*  mReportDlg;
    zchxRadarChannelReportCtrlDlg*  mFanCtrlDlg;
};

#endif // ZCHXRADARUI_H
