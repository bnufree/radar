#ifndef ZCHXRADARFILTERAREASETTINGDLG_H
#define ZCHXRADARFILTERAREASETTINGDLG_H

#include <QDialog>
#include "zchxdatadef.h"

namespace Ui {
class zchxRadarFilterAreaSettingDlg;
}

class zchxRadarFilterAreaSettingDlg : public QDialog
{
    Q_OBJECT

public:
    explicit zchxRadarFilterAreaSettingDlg(QWidget *parent = 0);
    ~zchxRadarFilterAreaSettingDlg();
    static zchxCommon::zchxArea   getAreaFromPath(const std::vector<std::pair<double, double> > &path);
public slots:
    void    setCurFilterData(const zchxCommon::zchxFilterArea& data);
    void    recvNewFilterArea(const std::vector<std::pair<double, double> > &path);

signals:
    void    signalNewSetting(const QJsonValue& bytes);

private slots:
    void on_filter_type_cbx_currentIndexChanged(int index);

    void on_edit_clicked();

    void on_look_clicked();

    void on_del_clicked();

    void on_ok_clicked();

private:
    Ui::zchxRadarFilterAreaSettingDlg *ui;
    zchxCommon::zchxFilterArea              mCurData;
};

#endif // ZCHXRADARFILTERAREASETTINGDLG_H
