#include <QtGui/QApplication>
#include "maindevicewindow.h"

#include <acewrapper/ace_string.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	device_emulator::MainDeviceWindow w;

    w.initial_update();
    w.show();
    return a.exec();
}
