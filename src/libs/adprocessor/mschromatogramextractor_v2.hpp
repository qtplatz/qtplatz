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
    class MSPeakInfoItem;
    class ProcessMethod;
}

namespace adprocessor {

    namespace v2 {

        class  ADPROCESSORSHARED_EXPORT MSChromatogramExtractor {
            
            MSChromatogramExtractor( const MSChromatogramExtractor& ) = delete;
            MSChromatogramExtractor& operator = ( const MSChromatogramExtractor& ) = delete;

        public:
            ~MSChromatogramExtractor();
            MSChromatogramExtractor( const adcontrols::LCMSDataset * );

            bool operator()( std::vector< std::shared_ptr< adcontrols::Chromatogram > >&
                             , const adcontrols::ProcessMethod&
                             , std::function<bool( size_t, size_t )> progress );

            bool operator()( std::vector< std::shared_ptr< adcontrols::Chromatogram > >&
                             , adcontrols::hor_axis
                             , const std::vector< std::pair< int, adcontrols::MSPeakInfoItem > >& ranges
                             , std::function<bool( size_t, size_t )> progress );
        
        private:
            
            bool loadSpectra( const adcontrols::ProcessMethod *
                              , std::function<bool( size_t, size_t )> progress );
            
            class impl;
            impl * impl_;
        };
    }
}

