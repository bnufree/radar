#ifndef ZCHXRADARBASESETTINGDLG_H
#define ZCHXRADARBASESETTINGDLG_H

#include <QDialog>
#include "zchxdatadef.h"

namespace Ui {
class zchxRadarBaseSettingDlg;
}

class zchxRadarBaseSettingDlg : public QDialog
{
    Q_OBJECT

public:
    explicit zchxRadarBaseSettingDlg(const zchxCommon::zchxRadarDeviceBaseSetting& setting = zchxCommon::zchxRadarDeviceBaseSetting(), QWidget *parent = 0);
    ~zchxRadarBaseSettingDlg();
public slots:
    void slotSetPos(double lat, double lon);
signals:
    void signalnewSetting(const QJsonValue& setting);
    void signalGetPosFromEcdis();

private slots:
    void on_ok_clicked();

    void on_cancel_clicked();

    void on_pos_from_ecdis_clicked();

private:
    Ui::zchxRadarBaseSettingDlg *ui;
    zchxCommon::zchxRadarDeviceBaseSetting mSetting;
};

#endif // ZCHXRADARBASESETTINGDLG_H
