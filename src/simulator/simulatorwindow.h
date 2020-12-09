#ifndef SIMULATORWINDOW_H
#define SIMULATORWINDOW_H

#include <QMainWindow>

namespace Ui {
class SimulatorWindow;
}

class SimulatorWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SimulatorWindow(QWidget *parent = 0);
    ~SimulatorWindow();

private slots:
    void on_browse_clicked();

    void on_start_clicked();

private:
    Ui::SimulatorWindow *ui;
};

#endif // SIMULATORWINDOW_H
