#include "simulatorwindow.h"
#include "ui_simulatorwindow.h"
#include <QButtonGroup>
#include "zchxradardatareceiver.h"
#include "zchxsimulatethread.h"
#include <QFileDialog>
#include <zchxradardatasender.h>

SimulatorWindow::SimulatorWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SimulatorWindow),
    mDataFinder(0)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("数据采集与模拟"));
    QButtonGroup* grp = new QButtonGroup(this);
    grp->setExclusive(true);
    grp->addButton(ui->collect, 1);
    grp->addButton(ui->fake, 2);
    connect(grp, (void(QButtonGroup::*)(int, bool))&QButtonGroup::buttonToggled, this, [=](int id, bool sts){
        if(id == 2)
        {
            slotStopReceiver();
            ui->fake_frame->setVisible(true);
        } else
        {
            ui->fake_frame->setVisible(false);
            slotStopSimulator();
        }
    });
    ui->collect->setChecked(true);
    mStartFlag = false;
}

SimulatorWindow::~SimulatorWindow()
{
    delete ui;
}

void SimulatorWindow::slotRecvResult(const QList<zchxRadarDev> &list, const QString &ip)
{
    mRadarList.clear();
    for(int i=0; i<list.size(); i++)
    {
        ZCHXRadarDataReceiver* server = new ZCHXRadarDataReceiver(list[i], ip, this);
        mRadarList.append(server);
        server->slotPrintRecvData(true);
    }
}

void SimulatorWindow::slotStopDadaFinder()
{
    if(mDataFinder)
    {
        delete mDataFinder;
        mDataFinder = 0;
    }
}

void SimulatorWindow::slotStopReceiver()
{
    slotStopDadaFinder();
    foreach (ZCHXRadarDataReceiver* server, mRadarList) {
        server->slotPrintRecvData(false);
        server->deleteLater();
    }
    mRadarList.clear();
}

void SimulatorWindow::slotStopSimulator()
{
    foreach (zchxSimulateThread* thread, mSimulateThreadList) {
        thread->setCancel(true);
        thread->deleteLater();
    }
    mSimulateThreadList.clear();

    foreach (ZCHXRadarDataSender* thread, mSenderList) {
        delete thread;
    }
    mSenderList.clear();
}

void SimulatorWindow::slotStartReceiver()
{
    mDataFinder = new zchxRadarDataFinder(this);
    connect(mDataFinder, SIGNAL(signalSendResult(QList<zchxRadarDev>,QString)),
            this, SLOT(slotRecvResult(QList<zchxRadarDev>,QString)));
}

void SimulatorWindow::slotStartSimulator()
{
    QString filePath = ui->fake_video_path->text();
    if(filePath > 0)
    {
        zchxSimulateThread* thread = new zchxSimulateThread(filePath, this);
        mSimulateThreadList.append(thread);
        ZCHXRadarDataSender *sender = new ZCHXRadarDataSender("236.6.7.0", 6670);
        connect(thread, SIGNAL(signalSendContents(QByteArray,int)), sender, SLOT(slotRecvSendData(QByteArray, int)));
        mSenderList.append(sender);
    }

    filePath = ui->fake_report_path->text();
    if(filePath > 0)
    {
        zchxSimulateThread* thread = new zchxSimulateThread(filePath, this);
        mSimulateThreadList.append(thread);
        ZCHXRadarDataSender *sender = new ZCHXRadarDataSender("236.6.7.1", 6671);
        connect(thread, SIGNAL(signalSendContents(QByteArray,int)), sender, SLOT(slotRecvSendData(QByteArray, int)));
        mSenderList.append(sender);
    }

    foreach (zchxSimulateThread* thread, mSimulateThreadList) {
        thread->start();
    }

}


void SimulatorWindow::on_start_clicked()
{
    if(!mStartFlag)
    {
        mStartFlag = true;
        if(ui->collect->isChecked())
        {
            slotStartReceiver();
        } else
        {
            slotStartSimulator();
        }
        ui->start->setText(QStringLiteral("结束"));
    } else
    {
        ui->start->setText(QStringLiteral("开始"));
        if(ui->collect->isChecked())
        {
            slotStopReceiver();
        } else
        {
            slotStopSimulator();
        }
        mStartFlag = false;
    }


}

void SimulatorWindow::on_video_browse_clicked()
{
    QString path = QFileDialog::getExistingDirectory();
    if(path.size() > 0) ui->fake_video_path->setText(path);
}

void SimulatorWindow::on_report_browse_clicked()
{
    QString path = QFileDialog::getExistingDirectory();
    if(path.size() > 0) ui->fake_report_path->setText(path);
}
