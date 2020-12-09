#ifndef ZCHXLOGINDLG_H
#define ZCHXLOGINDLG_H

#include <QDialog>

namespace Ui {
class zchxLoginDlg;
}

class zchxLoginDlg : public QDialog
{
    Q_OBJECT

public:
    explicit zchxLoginDlg(const QString& host, int port, QWidget *parent = 0);
    ~zchxLoginDlg();

signals:
    void    signalNewHostAndPort(const QString& host, int port);

private slots:
    void on_ok_clicked();

    void on_cancel_clicked();

private:
    Ui::zchxLoginDlg *ui;
};

#endif // ZCHXLOGINDLG_H
