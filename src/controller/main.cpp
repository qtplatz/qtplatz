#include <QtGui/QApplication>
#include "maincontrollerwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainControllerWindow w;
    w.show();

    return a.exec();
}
