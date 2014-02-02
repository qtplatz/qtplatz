// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#ifndef DATAPROCHANDLER_H
#define DATAPROCHANDLER_H

#include <string>
#include <vector>

namespace adcontrols {
    class MassSpectrum;
    class Chromatogram;
    class MSCalibrateResult;
	class MSCalibration;
    class MSAssignedMasses;
    class MSPeakInfo;
    class PeakResult;

    class CentroidMethod;
    class IsotopeMethod;
    class MSCalibrateMethod;
    class PeakMethod;
}

namespace portfolio {
    class Folium;
}

namespace dataproc {

    class DataprocHandler {
    public:
        DataprocHandler();

        static bool doCentroid( adcontrols::MSPeakInfo&
                                , adcontrols::MassSpectrum& res
                                , const adcontrols::MassSpectrum& profile
                                , const adcontrols::CentroidMethod& );

        static bool doIsotope( adcontrols::MassSpectrum& res, const adcontrols::IsotopeMethod& );

        static bool doMSCalibration( adcontrols::MSCalibrateResult& res
                                     , adcontrols::MassSpectrum& centroid
                                     , const adcontrols::MSCalibrateMethod& );

        static bool doMSCalibration( adcontrols::MSCalibrateResult& res
                                     , adcontrols::MassSpectrum& centroid
                                     , const adcontrols::MSCalibrateMethod&
                                     , const adcontrols::MSAssignedMasses& );
        static bool compute_polynomials( adcontrols::MSCalibrateResult& res, const adcontrols::MSAssignedMasses& );

        static bool doFindPeaks( adcontrols::PeakResult&, const adcontrols::Chromatogram& , const adcontrols::PeakMethod& ); 
        static bool doAnnotateAssignedPeaks( adcontrols::MassSpectrum& centroid
                                             , const adcontrols::MSAssignedMasses& );

        static bool apply_calibration( adcontrols::MassSpectrum&, const adcontrols::MSCalibration& );
        static bool reverse_copy( adcontrols::MSPeakInfo&, const adcontrols::MassSpectrum& );

    };

}

#endif // DATAPROCHANDLER_H
