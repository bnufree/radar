#ifndef ZCHXRADARPARSESETTINGDLG_H
#define ZCHXRADARPARSESETTINGDLG_H

#include <QDialog>
#include "zchxdatadef.h"

namespace Ui {
class zchxRadarParseSettingDlg;
}

class zchxRadarParseSettingDlg : public QDialog
{
    Q_OBJECT

public:
    explicit zchxRadarParseSettingDlg(QWidget *parent = 0) : QDialog(parent) {}
    explicit zchxRadarParseSettingDlg(const zchxCommon::zchxVideoParse& parse, QWidget *parent = 0);
    ~zchxRadarParseSettingDlg();
signals:
    void signalnewSetting(const QJsonValue& val);

private slots:
    void on_pushButton_clicked();

private:
    Ui::zchxRadarParseSettingDlg *ui;
    zchxCommon::zchxVideoParse      mParse;
};

#endif // ZCHXRADARPARSESETTINGDLG_H
