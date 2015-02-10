// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
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

#pragma once

#include <vector>
#include <cstddef>
#include <cstdint>
#include <functional>

namespace adportable {

    class spectrum_processor {
    public:
        struct areaFraction {
            size_t lPos;
            size_t uPos;
            double lFrac;
            double uFrac;
            areaFraction() : lPos(0), uPos(0), lFrac(0), uFrac(0) {}
        };
        static double tic( size_t nbrSamples, const int32_t * praw, double& dbase, double& sd, size_t N = 5 );
        static double tic( size_t nbrSamples, const double * praw, double& dbase, double& sd, size_t N = 5 );

        static void moving_average( size_t nbrSamples, double * result, const double * intens, size_t N = 5 );
        static void differentiation( size_t nbrSamples, double * result, const double * intens, size_t N = 5 );
        static double area( const double * beg, const double * end, double base );

        static bool getFraction( areaFraction&, const double * pMasses, size_t, double lMass, double hMass );
        static double area( const areaFraction&, double base, const double* pData, size_t nData );
        static double area( const areaFraction&, double base, const int32_t* pData, size_t nData );
    };

    struct peakinfo {
        size_t first;
        size_t second;
        double base;
        double mass;
        double time;
        double width;
        peakinfo( size_t x1, size_t x2, double _base )
            : first( x1 ), second( x2 ), base( _base ), mass(0), time(0), width(0) {}
    };

    class spectrum_peakfinder {
    public:
        enum WidthMethod { Constant, Proportional, TOF };
        spectrum_peakfinder( double pw = 0.1, double bw = 0, WidthMethod wm = Constant );
        size_t operator()( size_t nbrSamples, const double *pX, const double * pY );
        double peakwidth_;
        double atmz_;
        WidthMethod width_method_;
        std::vector< peakinfo > results_;
    };

    /** \brief simplified peak finder for any waveform w/ centroid
     * 
     * waveform_peakfinder ctor take a functor as an argument, which return peak width and 
     * number of samples equivalent to peak width for SG filter.
     */
    class waveform_peakfinder {
    public:
        
        struct peakinfo {
            size_t spos;
            size_t epos;
            size_t tpos;
            double height;
            double centreX;
            double xleft;
            double xright;
            peakinfo( size_t x1, size_t x2, size_t tp, double h = 0, double c = 0 )
                : spos( x1 ), epos( x2 ), tpos( tp ), height(h), centreX(c), xleft(0), xright(0)
                {}
        };

        struct parabola {
            size_t spos;
            size_t epos;
            size_t tpos;
            double height;
            double centreX;
            double a, b, c;  // a + bx + cx^2
            parabola( size_t x1 = 0, size_t x2 = 0, size_t tp = 0, double h = 0, double c = 0 )
                : spos( x1 ), epos( x2 ), tpos( tp ), height(h), centreX(c), a(0), b(0), c(0)
                {}
        };

        waveform_peakfinder( std::function< double( size_t idx, int& npeakw )> fpeakw );
        
        size_t operator()( std::function< double( size_t ) > fx
                           , const double * pY
                           , size_t beg, size_t end
                           , std::vector< waveform_peakfinder::peakinfo >& results );

        bool fit( std::function< double( size_t ) > fx
                  , const double * pY
                  , size_t spos
                  , size_t tpos
                  , size_t epos
                  , waveform_peakfinder::parabola& result );
        
        double dbase() const;
        double rms() const;        
    private:
        std::function< double( size_t idx, int& )> fpeakw_;
        double dbase_;
        double rms_;
    };
	
}

