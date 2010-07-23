#include <QtGui/QApplication>
#include "maincontrollerwindow.h"

#include "ace/Init_ACE.h"

#if defined ACE_WIN32
#  if defined _DEBUG
#     pragma comment(lib, "TAO_Utilsd.lib")
#     pragma comment(lib, "TAO_PId.lib")
#     pragma comment(lib, "TAO_PortableServerd.lib")
#     pragma comment(lib, "TAO_AnyTypeCoded.lib")
#     pragma comment(lib, "TAOd.lib")
#     pragma comment(lib, "ACEd.lib")
#  else
#     pragma comment(lib, "TAO_Utils.lib")
#     pragma comment(lib, "TAO_PI.lib")
#     pragma comment(lib, "TAO_PortableServer.lib")
#     pragma comment(lib, "TAO_AnyTypeCode.lib")
#     pragma comment(lib, "TAO.lib")
#     pragma comment(lib, "ACE.lib")
#  endif
#endif


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainControllerWindow w;

    w.on_initial_update();
    w.show();
    // w.mcast_init();
    return a.exec();
}
