/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#pragma once

#include <adcontrols/massspectrometer.hpp>
#include <cstddef>
#include <utility>
#include <vector>
#include <string>

namespace adcontrols {
    class MassSpectrum;
    class MSAssignedMasses;
    class MSCalibration;
	class MSProperty;
}

namespace dataproc {

    class mass_calibrator {
        std::vector< double > times_;
        std::vector< double > sqrtMz_;
        std::wstring ident_;
		//std::shared_ptr< adcontrols::ScanLaw > scanLaw_;

    public:
        mass_calibrator( const adcontrols::MSAssignedMasses&, const adcontrols::MSProperty& );

        inline size_t size() const { return times_.size(); }
        bool polfit( adcontrols::MSCalibration&, int nterm );
        double compute_mass( double time, int mode, const adcontrols::MSCalibration& );
    };

}


