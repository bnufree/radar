#include "simulatorwindow.h"
#include "ui_simulatorwindow.h"

SimulatorWindow::SimulatorWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SimulatorWindow)
{
    ui->setupUi(this);
}

SimulatorWindow::~SimulatorWindow()
{
    delete ui;
}

void SimulatorWindow::on_browse_clicked()
{

}

void SimulatorWindow::on_start_clicked()
{

}
