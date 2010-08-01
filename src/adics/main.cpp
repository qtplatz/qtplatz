//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include <QtGui/QApplication>
#include "mainwindow.h"
#include <acewrapper/acewrapper.h>
#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>

#  if defined _DEBUG
#     pragma comment(lib, "ACEd.lib")
#     pragma comment(lib, "TAOd.lib")
#     pragma comment(lib, "TAO_Utilsd.lib")
#     pragma comment(lib, "TAO_PId.lib")
#     pragma comment(lib, "TAO_PortableServerd.lib")
#     pragma comment(lib, "TAO_AnyTypeCoded.lib")
#     pragma comment(lib, "TAO_CosNamingd.lib")
#     pragma comment(lib, "adcontrollerd.lib")
#     pragma comment(lib, "adbrokerd.lib")
#     pragma comment(lib, "acewrapperd.lib")
#     pragma comment(lib, "adinterfaced.lib")
#  else
#     pragma comment(lib, "ACE.lib")
#     pragma comment(lib, "TAO.lib")
#     pragma comment(lib, "adcontroller.lib")
#     pragma comment(lib, "acewrapper.lib")
#  endif


int
main(int argc, char *argv[])
{
    AllocConsole();
    do {
        HANDLE stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        int hConHandle = _open_osfhandle( reinterpret_cast<intptr_t>(stdHandle), _O_TEXT );
        FILE * fp = _fdopen( hConHandle, "w" );
        *stdout = *fp;
    } while(0);

    do {
        HANDLE errHandle = GetStdHandle(STD_ERROR_HANDLE);
        int hErrHandle = _open_osfhandle( reinterpret_cast<intptr_t>(errHandle), _O_TEXT );
        FILE * fp = _fdopen( hErrHandle, "w" );
        *stderr = *fp;
    } while(0);
    
    std::cout << "Bonjour, ca va" << std::endl;

    acewrapper::instance_manager::initialize();

    QApplication a(argc, argv);

    MainWindow w;
    w.show();
    w.initial_update();
    return a.exec();
}
