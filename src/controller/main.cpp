#include <QtGui/QApplication>
#include "maincontrollerwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainControllerWindow w;

    w.on_initial_update();
    w.show();
    // w.mcast_init();
    return a.exec();
}
