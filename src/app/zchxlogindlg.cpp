#include "zchxlogindlg.h"
#include "ui_zchxlogindlg.h"

zchxLoginDlg::zchxLoginDlg(const QString& host, int port, QWidget *parent) :
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
}

zchxLoginDlg::~zchxLoginDlg()
{
    delete ui;
}

void zchxLoginDlg::on_ok_clicked()
{
    emit signalNewHostAndPort(ui->host->text().trimmed(), ui->port->value());
    close();
}

void zchxLoginDlg::on_cancel_clicked()
{
    close();
}
