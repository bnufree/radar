#include "radardataoutputsettings.h"
#include "ui_radardataoutputsettings.h"
#include <QSpinBox>
#include <QLineEdit>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

PortTopicSetting::PortTopicSetting(const QString& title, const QString& topic, int port, QWidget *parent):
    QWidget(parent),
    mTitle(title)

{
    QGridLayout* grid = new QGridLayout;
    this->setLayout(grid);
    int col = 0;
    grid->addWidget(new QLabel(title, this), 0, col, 1, 2);
    col += 2;
    grid->addWidget(new QLabel("Topic", this), 0, col++, 1, 1);
    mTopic = new QLineEdit(this);
    grid->addWidget(mTopic,0, col++, 1, 1);
    mTopic->setText(topic);
    grid->addWidget(new QLabel(QString::fromUtf8("端口"), this),0, col++, 1, 1);
    mPort = new QSpinBox(this);
    grid->addWidget(mPort,0, col++, 1, 1);
    mPort->setButtonSymbols(QAbstractSpinBox::NoButtons);
    mPort->setRange(0, 99999);
    mPort->setValue(port);

}

PortTopicSetting::~PortTopicSetting()
{

}

void PortTopicSetting::slotSetPortTopic(const QString &topic, int port)
{
    mTopic->setText(topic);
    mPort->setValue(port);
}

QString PortTopicSetting::getTopic() const
{
    return mTopic->text().trimmed();
}

int PortTopicSetting::getPort() const
{
    return mPort->value();
}




RadarDataOutputSettings::RadarDataOutputSettings(zchxCommon::zchxRadarServerCfg* cfg, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RadarDataOutputSettings),
    mCfg(cfg)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setFixedWidth(60 * 5);
    QGridLayout *layout = new QGridLayout;
    this->setLayout(layout);
    int row = 0;
    if(mCfg)
    {
        for(int i=0; i<mCfg->publish_list.size();i++)
        {
            zchxCommon::zchxPublishSetting setting = mCfg->publish_list[i];
            PortTopicSetting* widget = new PortTopicSetting(setting.desc, setting.topic, setting.port);
            widget->setProperty("id", setting.id);
            layout->addWidget(widget, row++, 0, 1, 5);
            mList.append(widget);
        }
    }
    QPushButton *ok = new QPushButton(QString::fromUtf8("确认"), this);
    connect(ok, SIGNAL(clicked(bool)), this, SLOT(slotOKClicked()));
    layout->addWidget(ok, row++, 4, 1, 1);
}

RadarDataOutputSettings::~RadarDataOutputSettings()
{
    delete ui;
}

void RadarDataOutputSettings::slotOKClicked()
{
    mCfg->publish_list.clear();
    for(int i=0; i<mList.size();i++)
    {
        PortTopicSetting *w = mList[i];
        if(!w) continue;
        int id = w->property("id").toInt();
        mCfg->publish_list.append(zchxCommon::zchxPublishSetting(id, w->getTopic(), w->getPort(), w->getTitle()));
    }
    emit signalResetPublishSettings();
    close();
}
