// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
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

#include "infitofprocessor_global.h"
#include <adprocessor/dataprocessor.hpp>
#include <vector>
#include <tuple>

namespace infitofwidgets { class ScanLawDialog; }
namespace adwidgets { class Progress; }

namespace infitofprocessor {
    
    class INFITOFPROCESSORSHARED_EXPORT CalibScanLaw {
    public:
        CalibScanLaw();
        ~CalibScanLaw();

        // [0]
        void operator()( std::shared_ptr< adprocessor::dataprocessor >
                         , std::shared_ptr< const adcontrols::MassSpectrum >, const std::pair< double, double >&, bool isTime );

        // [1] -- find mass reference peaks
        void operator()( std::shared_ptr< adprocessor::dataprocessor >, const portfolio::Folium& );

        // [2]
        bool computeScanLawTimeCourse( std::shared_ptr< adprocessor::dataprocessor >, std::shared_ptr< adwidgets::Progress > );

        // [3]
        bool loadScanLawTimeCourse( std::shared_ptr< adprocessor::dataprocessor > );

    private:
        void initialSetup( std::shared_ptr< adprocessor::dataprocessor >, infitofwidgets::ScanLawDialog& );
        std::vector< std::tuple< int64_t, double, double > > results_;
    };

}
