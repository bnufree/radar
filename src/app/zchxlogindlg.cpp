#include "zchxlogindlg.h"
#include "ui_zchxlogindlg.h"
#include "zchxregistorchecker.h"
#include <QDebug>

zchxLoginDlg::zchxLoginDlg(int mode, const QString& host, int port, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::zchxLoginDlg)
{
    ui->setupUi(this);
    setWindowTitle(QString::fromUtf8("登陆"));
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    ui->host->setText(host);
    ui->port->setValue(port);
    ui->cancel->setVisible(false);
    setMode(mode);
}

void zchxLoginDlg::setMode(int mode)
{
    mMode = mode;
    if(mMode == Dlg_Login)
    {
        ui->login_frame->setVisible(true);
        ui->regis_frame->setVisible(false);
        setWindowTitle(QString::fromUtf8("系统登陆"));
    } else
    {
        ui->login_frame->setVisible(false);
        ui->regis_frame->setVisible(true);
        setWindowTitle(QString::fromUtf8("系统注册"));
        zchxRegistorChecker checker;
        ui->machine->setText(checker.getMachineCode());
        ui->machine->setEnabled(true);
    }
}

void zchxLoginDlg::closeEvent(QCloseEvent *e)
{
    qDebug()<<"close event happened now...";
    QDialog::closeEvent(e);
}

QString zchxLoginDlg::getKey() const
{
    return ui->key->text().trimmed();
}

zchxLoginDlg::~zchxLoginDlg()
{
    qDebug()<<"destruct dlg now...";
    delete ui;
}

void zchxLoginDlg::on_ok_clicked()
{
    if(mMode == Dlg_Login)
    {
        emit signalNewHostAndPort(ui->host->text().trimmed(), ui->port->value());
    } else
    {
        zchxRegistorChecker checker;
        checker.startCheck(ui->key->text().trimmed());
    }
    accept();
}

void zchxLoginDlg::on_cancel_clicked()
{
    close();
}
