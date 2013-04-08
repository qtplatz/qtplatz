// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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
#include "mainwindow.h"

#if defined _DEBUG
#     pragma comment(lib, "adwplotd.lib") // static
#     pragma comment(lib, "qwtd.lib")
#     pragma comment(lib, "qtwrapperd.lib")
#     pragma comment(lib, "adcontrolsd.lib")
#else
#     pragma comment(lib, "adwplot.lib") // static
#     pragma comment(lib, "qwt.lib")
#     pragma comment(lib, "qtwrapper.lib")
#     pragma comment(lib, "adcontrols.lib")
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.resize( 540, 400 );
    w.show();

    return a.exec();
}
