// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
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

    /** \brief simplified peak finder for any waveform w/ centroid
     * 
     * waveform_peakfinder ctor take a functor as an argument, which return peak width and 
     * number of samples equivalent to peak width for SG filter.
     */
    namespace waveform_peakfinder {

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

        // fpeakw function: idx := bin number (index) on the waveform
        //                  npeakw := interger (count) value corresponding peak width equivalent
        //                  return := double value of peak width

        class peakfinder {
        public:
            peakfinder( std::function< double( size_t idx, int& npeakw )> fpeakw );

            /** \brief find peaks from waveform, intended input is the time-of-flight mass spectrum
             * 
             * Internally, it compute rms and baseline level by using TIC calculation algorithm.
             * Computed baseline level can be returned by dbase() method, and rms() method for RMS.
             * dbase is used for peak height determination, and rms is used for slope determination.
             */
            template< typename value_type = double >
            size_t operator()( std::function< double( size_t ) > fx
                               , const value_type * pY
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

        // due to Moment class is depend on 'double' y-array, cannot suppor other value_type for now.  to be fixed.
        template<> size_t peakfinder::operator()( std::function< double( size_t ) > fx, const double * pY
                                                  , size_t beg, size_t end, std::vector< waveform_peakfinder::peakinfo >& results );
    }
}

