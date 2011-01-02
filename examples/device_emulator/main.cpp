//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include <QtGui/QApplication>
#include "maindevicewindow.h"
#include "txtspectrum.h"
#include "devicefacade.h"
#include <qtwrapper/qstring.h>
//#include <acewrapper/ace_string.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	device_emulator::MainDeviceWindow w;

	device_emulator::TXTSpectrum sp;
    if ( sp.load( "PFTBA.txt" ) )
        device_emulator::singleton::device_facade::instance()->register_test_spectrum( sp );
    else if ( sp.load( (QApplication::applicationDirPath() + "/PFTBA.txt" ).toStdString() ) )
        device_emulator::singleton::device_facade::instance()->register_test_spectrum( sp );

    w.initial_update();
    w.show();
    return a.exec();
}
