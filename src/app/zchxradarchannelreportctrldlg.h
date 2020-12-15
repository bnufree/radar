#ifndef ZCHXRADARCHANNELREPORTCTRLDLG_H
#define ZCHXRADARCHANNELREPORTCTRLDLG_H

#include <QDialog>
#include "zchxdatadef.h"

class zchxRadarCtrlBtn;

namespace Ui {
class zchxRadarChannelReportCtrlDlg;
}

class zchxRadarChannelReportCtrlDlg : public QDialog
{
    Q_OBJECT

public:
    explicit zchxRadarChannelReportCtrlDlg(QWidget *parent = 0);
    ~zchxRadarChannelReportCtrlDlg();
    void appendBtn(zchxRadarCtrlBtn *btn);
    void setReportInfo(const zchxCommon::zchxRadarReportDataList& list, int radar, int channel);
    int  getRadarID() const {return mRadarID;}
    int  getChannelID() const {return mChannelID;}
    void setRadarType(int type);
    void updateRangefactor(double radius, double factor);
    void setControlFrameVisible(bool sts);
    void setFanFrameVisible(bool sts);
    void setRadarID(int id) {mRadarID = id;}
    void setChannelID(int id) {mChannelID = id;}

signals:
    void    signalCtrlValChanged(int type, QJsonValue val);

private slots:
    void on_openOrCloseRadarBtn_clicked();

    void on_fan1_clicked(bool checked);

    void on_fan2_clicked(bool checked);

    void on_fan3_clicked(bool checked);

    void on_fan4_clicked(bool checked);

    void on_angle1_valueChanged(int arg1);

    void on_width1_valueChanged(int arg1);

    void on_angle2_valueChanged(int arg1);

    void on_width2_valueChanged(int arg1);

    void on_angle3_valueChanged(int arg1);

    void on_width3_valueChanged(int arg1);

    void on_angle4_valueChanged(int arg1);

    void on_width4_valueChanged(int arg1);

    void on_resetRadar_clicked();

    void on_video_output_start_btn_clicked();

    void on_close_radar_btn_clicked();

private:
    Ui::zchxRadarChannelReportCtrlDlg *ui;
    QMap<int, zchxRadarCtrlBtn*>            mBtnList;
    int                                     mRadarID;
    int                                     mChannelID;

};

#endif // ZCHXRADARCHANNELREPORTCTRLDLG_H
