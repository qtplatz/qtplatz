/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#ifndef ASSIN_MASSES_HPP
#define ASSIN_MASSES_HPP

#include <cstddef>
#include <boost/noncopyable.hpp>

namespace adcontrols {
    class MSAssignedMasses;
    class MassSpectrum;
    class MSReferences;
}

namespace dataproc {

    class assign_masses : boost::noncopyable {
        double tolerance_;
        double threshold_;
    public:
        assign_masses( double tolerance, double threshold ) : tolerance_( tolerance ), threshold_( threshold ) {
        }
        static void make_color_array( unsigned char * colors, const adcontrols::MSAssignedMasses& assigned, std::size_t size );
        bool operator()( adcontrols::MSAssignedMasses&, const adcontrols::MassSpectrum&, const adcontrols::MSReferences&, int mode );

        //static int assign_peak_by_time( const adcontrols::MassSpectrum& centroid, double t, double tolerance = 3e-9);
        //void assign_peaks_by_time( adcontrols::MSAssignedMasses&
        //, const adcontrols::MassSpectrum& centroid, const adcontrols::MSAssignedMasses& );

    };

}

#endif // ASSIN_MASSES_HPP
