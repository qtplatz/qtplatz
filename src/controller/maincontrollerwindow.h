#ifndef MAINCONTROLLERWINDOW_H
#define MAINCONTROLLERWINDOW_H

#include <QMainWindow>

namespace Ui {
    class MainControllerWindow;
}

class MainControllerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainControllerWindow(QWidget *parent = 0);
    ~MainControllerWindow();

private:
    Ui::MainControllerWindow *ui;

private slots:
    void on_MainControllerWindow_destroyed();
    void on_connectButton_clicked();
};

#endif // MAINCONTROLLERWINDOW_H
