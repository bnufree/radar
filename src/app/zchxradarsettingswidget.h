#ifndef ZCHXRADARSETTINGSWIDGET_H
#define ZCHXRADARSETTINGSWIDGET_H

#include <QWidget>
#include "zchxradarbasesettingdlg.h"
#include "zchxradarparsesettingdlg.h"
#include "zchxradarchannelsettingdlg.h"
#include "zchxradarchannelreportctrldlg.h"

namespace Ui {
class zchxRadarSettingsWidget;
}

class zchxRadarSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit zchxRadarSettingsWidget(QWidget *parent = 0);
    ~zchxRadarSettingsWidget();

private:
    Ui::zchxRadarSettingsWidget *ui;
    zchxRadarBaseSettingDlg*            mBaseSettingDlg;
    zchxRadarParseSettingDlg*           mParseSettingDlg;
    zchxRadarChannelSettingDlg*         mChannelSettingDlg;
};

#endif // ZCHXRADARSETTINGSWIDGET_H
