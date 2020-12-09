#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <windows.h>
#include <tlhelp32.h>// for CreateToolhelp32Snapshot
#include <psapi.h>   // for GetModuleFileNameEx
#include <QTimer>
#include <QProcess>
#include <QDateTime>
#include <QSystemTrayIcon>
#include <QDir>

#define FORMAT_PATH(path) path.replace('\\','/').toLower()

const QString myAppName = "radar_client.exe";

bool KillProcess(DWORD ProcessId)
{
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE,FALSE,ProcessId);
    if(hProcess==NULL)
        return false;
    if(!TerminateProcess(hProcess,0))
        return false;
    return true;
}


MainWindow::MainWindow(const QString& app, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mAppName(app),
    mExeID(-1)
{
    ui->setupUi(this);
    if(mAppName.isEmpty()) mAppName = myAppName;
    mDogFile = mAppName;
    int index = mDogFile.indexOf(".exe");
    if(index >= 0)
    {
        mDogFile.replace(index, 4, ".txt");
    }
    setWindowFlags(Qt::SubWindow);

    QTimer *timer = new QTimer(this);
    timer->setInterval(3000 * 10);
    connect(timer, SIGNAL(timeout()), this, SLOT(slotTimeOut()));
    timer->start();
    slotTimeOut();
}

MainWindow::~MainWindow()
{
    if(mExeID > 0)
    {
        KillProcess(mExeID);
    }
    delete ui;
}

void MainWindow::slotTimeOut()
{
    if(mExeID == -1)
    {
        //程序还没有启动
        runApp();
        return;
    }

    //检查目标是死是活
    QDir dir(QApplication::applicationDirPath() + QString("/watchdog"));
    if(!dir.exists())
    {
        dir.mkpath(dir.path());
    }

    QString fileName = QString("%1/%2").arg(dir.path()).arg(mDogFile);
    QFileInfo fileInfo(fileName);
    //检查文件的更新时间
    QDateTime now = QDateTime::currentDateTime();
    if(fileInfo.lastModified().secsTo(now) >= 40)
    {
        //目标已经死了,关掉这个进程,然后重新启动
        KillProcess(mExeID);
        mExeID = -1;
        runApp();
    }
}
void MainWindow::runApp()
{
    if(mSub) delete mSub;
    mSub = new QProcess;
    mSub->start(mAppName);
    mExeID = mSub->processId();
}







