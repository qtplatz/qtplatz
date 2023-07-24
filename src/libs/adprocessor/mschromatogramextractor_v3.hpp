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
#include <boost/optional.hpp>
#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

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

namespace boost { namespace json { class object; } }

namespace adprocessor {

    class dataprocessor;
    class AutoTargetingCandidates;

    namespace v3 {

        class ADPROCESSORSHARED_EXPORT MSChromatogramExtractor {

            MSChromatogramExtractor( const MSChromatogramExtractor& ) = delete;
            MSChromatogramExtractor& operator = ( const MSChromatogramExtractor& ) = delete;

        public:
            ~MSChromatogramExtractor();
            MSChromatogramExtractor( const adcontrols::LCMSDataset *
                                     , dataprocessor * );

            // [0] Chromatograms from a list of mol in process method
            bool extract_by_mols( std::vector< std::shared_ptr< adcontrols::Chromatogram > >&
                                  , const adcontrols::ProcessMethod&
                                  , std::shared_ptr< const adcontrols::DataReader >
                                  , std::function<bool( size_t, size_t )> progress );

            bool extract_by_mols( std::vector< std::shared_ptr< adcontrols::Chromatogram > >&
                                  , const adcontrols::ProcessMethod&
                                  , std::shared_ptr< const adcontrols::DataReader >
                                  , const std::vector< AutoTargetingCandidates >&
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

            // [3] Chromatograms from targeting result json
            bool extract_by_json( std::vector< std::shared_ptr< adcontrols::Chromatogram > >&
                                  , const adcontrols::ProcessMethod&
                                  , std::shared_ptr< const adcontrols::DataReader >
                                  , const std::string& json
                                  , double width
                                  , adcontrols::hor_axis axis
                                  , std::function<bool( size_t, size_t )> progress );

            static boost::optional< double > computeIntensity( const adcontrols::MassSpectrum&
                                                               , adcontrols::hor_axis
                                                               , const std::pair< double, double >& );

            std::shared_ptr< const adcontrols::MassSpectrum > getMassSpectrum( double tR ) const;

            const std::vector< std::pair< int64_t, std::array<double, 2> > >& lkms() const;

            std::chrono::time_point< std::chrono::system_clock, std::chrono::nanoseconds > time_of_injection() const;

        private:

            bool loadSpectra( const adcontrols::ProcessMethod *
                              , std::shared_ptr< const adcontrols::DataReader >
                              , int fcn
                              , std::function<bool( size_t, size_t )> progress
                              , size_t
                              , size_t& );

            class impl;
            impl * impl_;
        };
    }
}
