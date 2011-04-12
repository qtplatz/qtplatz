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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <adcontrols/chromatogram.h>
#include <adcontrols/peak.h>
#include <adcontrols/peaks.h>
#include <adcontrols/massspectrum.h>
#include <adwplot/chromatogramwidget.h>
#include <adwplot/spectrumwidget.h>

#include <qwt_symbol.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>

#include <qwt_legend.h>
#include <qwt_text.h>

#include <cmath>
#include <boost/math/distributions/normal.hpp>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    // adwplot::SpectrumWidget * plot_ = new adwplot::SpectrumWidget( this );
    adwplot::ChromatogramWidget * plot_ = new adwplot::ChromatogramWidget( this );

    setContextMenuPolicy( Qt::NoContextMenu );
    setCentralWidget( plot_ );
    plot_->setTitle( L"Title 1" );

    boost::math::normal p1(50.0, 1.0);
    boost::math::normal p2(60.0, 1.2);

    adcontrols::Chromatogram c;
    do {
        std::vector<double> x, y;
        for ( int i = 0; i < 1000; ++i ) {
            double t = i * 0.1;
            x.push_back( t );
            y.push_back( boost::math::pdf(p1, t) + boost::math::pdf(p2, t) ); // probaility density function
        }
        c.resize( x.size() );
        c.setTimeArray( &x[0] );
        c.setIntensityArray( &y[0] );
        adcontrols::Peaks& peaks = c.peaks();
        adcontrols::Peak peak;
        peak.peakTime( x[500] );
        peak.peakHeight( y[500] );
        peak.startTime( peak.peakTime() - 1.0 );
        peak.endTime( peak.peakTime() + 1.0 );

        peaks.add( peak );

        peak.peakTime( x[600] );
        peak.peakHeight( y[600] );
        peak.startTime( peak.peakTime() - 1.0 );
        peak.endTime( peak.peakTime() + 1.0 );

        peaks.add( peak );

    } while(0);

    plot_->setData( c );

}

MainWindow::~MainWindow()
{
}
