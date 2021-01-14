#ifndef SIMULATORWINDOW_H
#define SIMULATORWINDOW_H

#include <QMainWindow>
#include "zchxradardatareceiver.h"

namespace Ui {
class SimulatorWindow;
}
class zchxRadarDataFinder;
class ZCHXRadarDataReceiver;
class zchxSimulateThread;
class ZCHXRadarDataSender;

class SimulatorWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SimulatorWindow(QWidget *parent = 0);
    ~SimulatorWindow();

public slots:
    void slotRecvResult(const QList<zchxRadarDev>& list, const QString& ip);
    void slotStopReceiver();
    void slotStopSimulator();
    void slotStartReceiver();
    void slotStartSimulator();
    void slotStopDadaFinder();

private slots:

    void on_start_clicked();

    void on_video_browse_clicked();

    void on_report_browse_clicked();

private:
    Ui::SimulatorWindow *ui;
    zchxRadarDataFinder*        mDataFinder;
    QList<ZCHXRadarDataReceiver*>  mRadarList;
    bool                            mStartFlag;
    QList<zchxSimulateThread*>  mSimulateThreadList;
    QList<ZCHXRadarDataSender*>  mSenderList;
};

#endif // SIMULATORWINDOW_H
