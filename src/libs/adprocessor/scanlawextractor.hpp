/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include "adprocessor_global.hpp"
#include <adcontrols/constants.hpp>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>

namespace adcontrols {

    class datafile;
    class CentroidMethod;
    class Chromatogram;
    class LCMSDataset;
    class MassSpectrum;
    class MSChromatogramMethod;
    class MSPeakInfo;
    class MSPeakInfoItem;
    class ProcessMethod;
    class DataReader;
}

namespace adprocessor {

    class dataprocessor;
    
    namespace v3 {

        class ADPROCESSORSHARED_EXPORT ScanLawExtractor {
            
            ScanLawExtractor( const ScanLawExtractor& ) = delete;
            ScanLawExtractor& operator = ( const ScanLawExtractor& ) = delete;
            
        public:
            ~ScanLawExtractor();
            ScanLawExtractor();

            bool operator()( std::shared_ptr< adprocessor::dataprocessor >
                             , const adcontrols::ProcessMethod&
                             , const std::string& objtext
                             , int proto
                             , std::function<bool( size_t, size_t )> progress );
            
        private:
            bool loadSpectra( std::shared_ptr< adprocessor::dataprocessor >
                              , const adcontrols::ProcessMethod *
                              , std::shared_ptr< const adcontrols::DataReader >
                              , int fcn
                              , std::function<bool( size_t, size_t )> progress );

            bool doCentroid(adcontrols::MassSpectrum& centroid
                            , const adcontrols::MassSpectrum& profile
                            , const adcontrols::CentroidMethod& m );
        };
    }
}

