#include "zchxvideocolorsettingwidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "zchxvideotermcolorsettingwidget.h"
#include <QPushButton>
#include <QDebug>

zchxVideoColorSettingWidget::zchxVideoColorSettingWidget(int count, const QVariantList& list, QWidget *parent) : QWidget(parent)
{
    mCount = count;
    mColorList = list;
    QVBoxLayout *vlay = new QVBoxLayout;
    this->setLayout(vlay);

    while (mColorList.size() < mCount)
    {
        mColorList.append(QVariant("#ffffff"));
    }

    mColorList = mColorList.mid(0, mCount);

    int height  = 0;
    for(int i=0; i<mColorList.size(); i++)
    {
        zchxVideoTermColorSettingWidget* w = new zchxVideoTermColorSettingWidget(i+1, mColorList[i].toString(), this);
        vlay->addWidget(w);
        mList.append(w);
        height += w->height() * 1.1;

    }

    height += 40;

    //添加确认按钮
    QHBoxLayout* hlay = new QHBoxLayout;
    vlay->addLayout(hlay);
    hlay->addSpacerItem(new QSpacerItem(10, 10));
    QPushButton * btn = new QPushButton(QString::fromUtf8("确认"), this);
    btn->setFixedWidth(60);
    hlay->addWidget(btn);
    connect(btn, SIGNAL(clicked(bool)), this, SIGNAL(signalSetFinished()));
    this->setFixedSize(200, height);
}


zchxVideoColorSettingWidget::~zchxVideoColorSettingWidget()
{
    qDebug()<<"term color delete now";
}

QVariantList zchxVideoColorSettingWidget::getColorList() const
{
    QVariantList list;
    foreach (zchxVideoTermColorSettingWidget* w, mList) {
        list.append(QVariant(w->colorName()));
    }
    return list;
}
