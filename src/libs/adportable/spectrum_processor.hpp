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

#pragma once

#include <vector>

namespace adportable {

    class spectrum_processor {
    public:
        static double tic( unsigned int nbrSamples, const long * praw, double& dbase, double& sd );
        static double tic( unsigned int nbrSamples, const double * praw, double& dbase, double& sd );
        static size_t findpeaks( size_t nbrSamples, const double *pX, const double * pY, std::vector< std::pair<int, int> >&, size_t N = 5 );
        static void smoozing( size_t nbrSamples, double * result, const double * intens, size_t N = 5 );
        static void differentiation( size_t nbrSamples, double * result, const double * intens, size_t N = 5 );
        static double area( const double * beg, const double * end, double base );
    };

    struct peakinfo {
        size_t first;
        size_t second;
        double base;
        peakinfo( size_t x1, size_t x2, double _base ) : first( x1 ), second( x2 ), base( _base ) {}
    };

    class spectrum_peakfinder {
    public:
        enum WidthMethod { Constant, Proportional, TOF };
        spectrum_peakfinder( double pw = 0.1, double bw = 50, WidthMethod wm = Constant );
        size_t operator()( size_t nbrSamples, const double *pX, const double * pY );

        double peakwidth_;
        double atmz_;
        double baseline_width_;
        std::vector< double > pdebug_;   // for internal debug
        WidthMethod width_method_;
        std::vector< peakinfo > results_;
    };
	
}

