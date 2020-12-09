#ifndef RADARDATAOUTPUTSETTINGS_H
#define RADARDATAOUTPUTSETTINGS_H

#include <QWidget>
#include "zchxdatadef.h"
namespace Ui {
class RadarDataOutputSettings;
}

class QLineEdit;
class QSpinBox;
class PortTopicSetting : public QWidget
{
    Q_OBJECT
public:
    explicit PortTopicSetting(const QString& title, const QString& topic, int port, QWidget *parent = 0);
    ~PortTopicSetting();
    QString getTopic() const ;
    int     getPort() const;
    QString getTitle() const {return mTitle;}
public slots:
    void slotSetPortTopic(const QString& topic, int port);

private:
    QLineEdit           *mTopic;
    QSpinBox            *mPort;
    QString             mTitle;

};

class RadarDataOutputSettings : public QWidget
{
    Q_OBJECT

public:
    explicit RadarDataOutputSettings(zchxCommon::zchxRadarServerCfg* cfg, QWidget *parent = 0);
    ~RadarDataOutputSettings();
signals:
    void signalResetPublishSettings();
private slots:
    void slotOKClicked();

private:
    Ui::RadarDataOutputSettings *ui;
    zchxCommon::zchxRadarServerCfg* mCfg;
    QList<PortTopicSetting*>     mList;

};

#endif // RADARDATAOUTPUTSETTINGS_H
