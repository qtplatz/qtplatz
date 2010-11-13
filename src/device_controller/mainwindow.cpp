#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_start_stop_clicked(bool checked)
{

}

void MainWindow::on_pushButton_connect_clicked(bool checked)
{

}

void MainWindow::on_pushButton_inject_clicked(bool checked)
{

}
