#ifndef ZCHXVIDEOTERMCOLORSETTINGWIDGET_H
#define ZCHXVIDEOTERMCOLORSETTINGWIDGET_H

#include <QWidget>

namespace Ui {
class zchxVideoTermColorSettingWidget;
}

class zchxVideoTermColorSettingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit zchxVideoTermColorSettingWidget(int id, const QString& color_name, QWidget *parent = 0);
    ~zchxVideoTermColorSettingWidget();
    int id() const {return mID;}
    QString colorName() const {return mColorName;}

private slots:
    void on_color_sel_btn_clicked();

private:
    Ui::zchxVideoTermColorSettingWidget *ui;
    int         mID;
    QString     mColorName;
};

#endif // ZCHXVIDEOTERMCOLORSETTINGWIDGET_H
