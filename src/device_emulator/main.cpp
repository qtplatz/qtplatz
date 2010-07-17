#include <QtGui/QApplication>
#include "maindevicewindow.h"

#include <acewrapper/ace_string.h>
#include <ace/INET_Addr.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainDeviceWindow w;
    w.initial_update();
    w.show();
    return a.exec();
}
