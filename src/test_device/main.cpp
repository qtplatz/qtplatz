#include <QtGui/QApplication>
#include "maindevicewindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainDeviceWindow w;
    w.show();

    return a.exec();
}
