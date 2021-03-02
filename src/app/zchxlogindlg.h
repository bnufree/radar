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
    enum DlgMode{
        Dlg_Login = 0,
        Dlg_Reg = 1,
    };

    explicit zchxLoginDlg(int mode, const QString& host = QString(), int port = 0, QWidget *parent = 0);
    ~zchxLoginDlg();

    void setMode(int mode);
    QString getKey() const;

protected:
    void closeEvent(QCloseEvent* e);

signals:
    void    signalNewHostAndPort(const QString& host, int port);
    void    signalNewRegisterKey(const QString& key);

private slots:
    void on_ok_clicked();

    void on_cancel_clicked();

private:
    Ui::zchxLoginDlg *ui;
    int     mMode;
};

#endif // ZCHXLOGINDLG_H
