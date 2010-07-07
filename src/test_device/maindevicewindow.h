#ifndef MAINDEVICEWINDOW_H
#define MAINDEVICEWINDOW_H

#include <QMainWindow>

namespace Ui {
    class MainDeviceWindow;
}

class MainDeviceWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainDeviceWindow(QWidget *parent = 0);
    ~MainDeviceWindow();

private:
    Ui::MainDeviceWindow *ui;
};

#endif // MAINDEVICEWINDOW_H
