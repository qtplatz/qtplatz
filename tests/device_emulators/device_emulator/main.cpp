// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#include <QtWidgets/QApplication>
#include "maindevicewindow.h"
#include "txtspectrum.h"
#include "devicefacade.h"
#include <QtCore/qstring.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	device_emulator::MainDeviceWindow w;

	device_emulator::TXTSpectrum sp;

    if ( sp.load( "PFTBA.txt" ) )
        device_emulator::singleton::device_facade::instance()->register_test_spectrum( sp );
    else if ( sp.load( (QApplication::applicationDirPath() + "/PFTBA.txt" ).toStdString() ) )
        device_emulator::singleton::device_facade::instance()->register_test_spectrum( sp );

    if ( sp.load3( "Xe.txt" ) )
        device_emulator::singleton::device_facade::instance()->register_test_spectrum( sp );
    else if ( sp.load3( (QApplication::applicationDirPath() + "/Xe.txt" ).toStdString() ) )
        device_emulator::singleton::device_facade::instance()->register_test_spectrum( sp );

    w.initial_update();
    w.show();
    return a.exec();
}
