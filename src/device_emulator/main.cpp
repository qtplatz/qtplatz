#include <QtGui/QApplication>
#include "maindevicewindow.h"

#include <acewrapper/ace_string.h>
#include <ace/INET_Addr.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    ACE_INET_Addr addr;
    std::wstring wstr = acewrapper::wstring(addr);
    std::string sstr = acewrapper::string(addr);

    MainDeviceWindow w;
    w.show();
    return a.exec();
}
