#include "maincontrollerwindow.h"
#include "ui_maincontrollerwindow.h"

MainControllerWindow::MainControllerWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainControllerWindow)
{
    ui->setupUi(this);
}

MainControllerWindow::~MainControllerWindow()
{
    delete ui;
}

void MainControllerWindow::on_connectButton_clicked()
{

}

void MainControllerWindow::on_MainControllerWindow_destroyed()
{

}
