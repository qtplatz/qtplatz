/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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
    namespace v3 {

        class ADPROCESSORSHARED_EXPORT MSChromatogramExtractor {
            
            MSChromatogramExtractor( const MSChromatogramExtractor& ) = delete;
            MSChromatogramExtractor& operator = ( const MSChromatogramExtractor& ) = delete;
            
        public:
            ~MSChromatogramExtractor();
            MSChromatogramExtractor( const adcontrols::LCMSDataset * );
            
            // [0] Chromatograms from a list of mol in process method
            bool extract_by_mols( std::vector< std::shared_ptr< adcontrols::Chromatogram > >&
                                  , const adcontrols::ProcessMethod&
                                  , std::shared_ptr< const adcontrols::DataReader >
                                  //, int fcn
                                  , std::function<bool( size_t, size_t )> progress );

            // [1] Chromatograms from centroid result
            bool extract_by_peak_info( std::vector< std::shared_ptr< adcontrols::Chromatogram > >& vec
                                       , const adcontrols::ProcessMethod&
                                       , std::shared_ptr< const adcontrols::MSPeakInfo > pkinfo
                                       , std::shared_ptr< const adcontrols::DataReader > reader                        
                                       , std::function<bool( size_t, size_t )> progress );
            
            // [2] Chromatograms from specified m/z or time range
            bool extract_by_axis_range( std::vector< std::shared_ptr< adcontrols::Chromatogram > >& vec
                                        , const adcontrols::ProcessMethod&
                                        , std::shared_ptr< const adcontrols::DataReader >
                                        , int fcn
                                        , adcontrols::hor_axis axis
                                        , const std::pair< double, double >& range
                                        , std::function<bool( size_t, size_t )> progress );

            static bool computeIntensity( double& y, const adcontrols::MassSpectrum&, adcontrols::hor_axis, const std::pair< double, double >& );
            
        private:
            bool loadSpectra( const adcontrols::ProcessMethod *
                              , std::shared_ptr< const adcontrols::DataReader >
                              , int fcn
                              , std::function<bool( size_t, size_t )> progress );

            class impl;
            impl * impl_;
        };
    }
}

