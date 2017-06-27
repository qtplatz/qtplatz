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

#include <array>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <adcontrols/quanresponse.hpp>

namespace adcontrols {
    class CentroidMethod;
    class QuanCompounds;
    class MassSpectrum;
    class MSPeakInfo;
    class MSLockMethod;
    class ProcessMethod;
    class QuanSample;
    namespace lockmass { class mslock; }
}

namespace adprocessor {
    class dataprocessor;
}

namespace quan {

    class QuanDataWriter;

    struct FindCompounds {
    public:
        const adcontrols::CentroidMethod& cm_;
        const adcontrols::QuanCompounds& compounds_;
        const double tolerance_;

        std::array< std::shared_ptr< adcontrols::MassSpectrum >, 2 > profile_;  // histogram|profile
        std::array< std::shared_ptr< adcontrols::MassSpectrum >, 2 > centroid_; // centroid (counting|profile)
        std::array< std::shared_ptr< adcontrols::MSPeakInfo >, 2 > pkinfo_;     // pkinfo (counting|profile)
        std::shared_ptr< adcontrols::lockmass::mslock > mslock_;

        std::map< boost::uuids::uuid, adcontrols::QuanResponse > responses_;
        
        FindCompounds( const adcontrols::QuanCompounds& cmpds
                       , const adcontrols::CentroidMethod& cm
                       , double tolerance );
        
        bool doCentroid( std::shared_ptr< adprocessor::dataprocessor > dp
                         , std::shared_ptr< adcontrols::MassSpectrum > ms
                         , bool isCounting );
        
        bool doMSLock( const adcontrols::MSLockMethod& m, bool isCounting );
        
        bool operator()( std::shared_ptr< adprocessor::dataprocessor > dp, bool isCounting );

        void write( std::shared_ptr< QuanDataWriter > writer
                    , const std::wstring& name
                    , std::shared_ptr< const adcontrols::ProcessMethod > pm
                    , adcontrols::QuanSample& sample
                    , bool isCounting
                    , std::shared_ptr< adprocessor::dataprocessor > dp );
    };
}
