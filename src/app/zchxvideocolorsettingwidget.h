#ifndef ZCHXVIDEOCOLORSETTINGWIDGET_H
#define ZCHXVIDEOCOLORSETTINGWIDGET_H

#include <QWidget>
#include <QVariant>

class zchxVideoTermColorSettingWidget;

class zchxVideoColorSettingWidget : public QWidget
{
    Q_OBJECT
public:
    explicit zchxVideoColorSettingWidget(int count, const QVariantList& list, QWidget *parent = 0);
    ~zchxVideoColorSettingWidget();
    QVariantList  getColorList() const;
signals:
    void signalSetFinished();
public slots:

private:
    int mCount;
    QVariantList  mColorList;
    QList<zchxVideoTermColorSettingWidget*>     mList;
};

#endif // ZCHXVIDEOCOLORSETTINGWIDGET_H
