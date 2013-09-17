/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#ifndef CALIBRATE_MASSES_HPP
#define CALIBRATE_MASSES_HPP

#include <cstddef>
#include <utility>
#include <vector>

namespace adcontrols {
    class MassSpectrum;
    class MSAssignedMasses;
    class MSCalibration;
}

namespace dataproc {

    class mass_calibrator {
        std::vector< double > times_;
        std::vector< double > sqrtMz_;
        std::wstring ident_;
    public:
        mass_calibrator();
        mass_calibrator( const adcontrols::MSAssignedMasses& );
        inline const size_t size() const { return times_.size(); }
        mass_calibrator& operator << ( const std::pair< double, double >& );
        bool compute( adcontrols::MSCalibration&, int nterm );
        static double compute_mass( double time, const adcontrols::MSCalibration& );
    };

}

#endif // CALIBRATE_MASSES_HPP
