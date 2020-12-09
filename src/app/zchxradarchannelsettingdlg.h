#ifndef ZCHXRADARCHANNELSETTINGDLG_H
#define ZCHXRADARCHANNELSETTINGDLG_H

#include <QDialog>
#include "zchxdatadef.h"

namespace Ui {
class zchxRadarChannelSettingDlg;
}

class zchxRadarChannelSettingDlg : public QDialog
{
    Q_OBJECT

public:
    explicit zchxRadarChannelSettingDlg(const zchxCommon::zchxRadarChannel& channel, QWidget *parent = 0);
    ~zchxRadarChannelSettingDlg();

signals:
    void signalNewSetting(const QJsonValue& val);

private slots:
    void on_ok_clicked();

    void on_cancel_clicked();

private:
    Ui::zchxRadarChannelSettingDlg *ui;
    zchxCommon::zchxRadarChannel    mChannel;
};

#endif // ZCHXRADARCHANNELSETTINGDLG_H
