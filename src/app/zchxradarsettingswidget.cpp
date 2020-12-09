#include "zchxradarsettingswidget.h"
#include "ui_zchxradarsettingswidget.h"


zchxRadarSettingsWidget::zchxRadarSettingsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::zchxRadarSettingsWidget)
{
    ui->setupUi(this);
}

zchxRadarSettingsWidget::~zchxRadarSettingsWidget()
{
    delete ui;
}
