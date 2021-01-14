#include "zchxvideotermcolorsettingwidget.h"
#include "ui_zchxvideotermcolorsettingwidget.h"
#include <QColorDialog>

zchxVideoTermColorSettingWidget::zchxVideoTermColorSettingWidget(int id, const QString& color_name, QWidget *parent) :
    QWidget(parent),
    mID(id),
    mColorName(color_name),
    ui(new Ui::zchxVideoTermColorSettingWidget)
{
    ui->setupUi(this);
    ui->term_name->setText(QString::fromUtf8("周期%1").arg(mID));
    ui->color_sel_btn->setToolTip(QString::fromUtf8("点击设定回波颜色"));
    ui->color_sel_btn->setAutoFillBackground(true);
    ui->color_sel_btn->setStyleSheet(QString("border:1px solid gray;background-color:%1").arg(mColorName));
//    QPalette palette = ui->color_sel_btn->palette();
//    palette.setColor(QPalette::Background, QColor(mColorName));
//    ui->color_sel_btn->setPalette(palette);
}

zchxVideoTermColorSettingWidget::~zchxVideoTermColorSettingWidget()
{
    delete ui;
}

void zchxVideoTermColorSettingWidget::on_color_sel_btn_clicked()
{
    QColor color = QColorDialog::getColor();
    if(color.isValid())
    {
        mColorName = color.name();
        ui->color_sel_btn->setStyleSheet(QString("border:1px solid gray;background-color:%1").arg(mColorName));
    }
}
