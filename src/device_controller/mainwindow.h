#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

private slots:
    void on_pushButton_inject_clicked(bool checked);
    void on_pushButton_connect_clicked(bool checked);
    void on_pushButton_start_stop_clicked(bool checked);
};

#endif // MAINWINDOW_H
