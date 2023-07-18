/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "centroid_processor.hpp"
#include <adcontrols/processmethod.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/centroidprocess.hpp>
#include <adcontrols/segment_wrapper.hpp>

namespace adprocessor {

    class centroid_processor::impl {
    public:
        std::unique_ptr< adcontrols::CentroidMethod > m_;
        impl() : m_( std::make_unique< adcontrols::CentroidMethod >() ) {
        }
    };
}

using namespace adprocessor;

centroid_processor::~centroid_processor()
{
}

centroid_processor::centroid_processor() : impl_( std::make_unique< impl >() )
{
}

centroid_processor::centroid_processor( const adcontrols::ProcessMethod& m )
    : impl_( std::make_unique< impl >() )
{
    setup( m );
}

void
centroid_processor::setup( const adcontrols::ProcessMethod& m )
{
    if ( auto cm = m.find< adcontrols::CentroidMethod >() ) {
        *impl_->m_ = *cm;
    }
}

const adcontrols::CentroidMethod&
centroid_processor::centroidMethod() const
{
    return *impl_->m_;
}

std::pair< std::shared_ptr< adcontrols::MassSpectrum >
           , std::shared_ptr< adcontrols::MSPeakInfo > >
centroid_processor::operator()( const adcontrols::MassSpectrum& profile ) const
{
    adcontrols::CentroidProcess peak_detector;
    bool result = false;

    auto pCentroid = std::make_shared< adcontrols::MassSpectrum >();
    auto pInfo = std::make_shared< adcontrols::MSPeakInfo >();

    int fcn(0);
    for ( auto& seg: adcontrols::segment_wrapper< const adcontrols::MassSpectrum >( profile ) ) {

        if ( fcn == 0 ) {
            peak_detector( *impl_->m_, seg );

            peak_detector.getCentroidSpectrum( *pCentroid );
            *pInfo = peak_detector.getPeakInfo();
            pInfo->setProtocol( 0, profile.numSegments() + 1 );
            pCentroid->setProtocol( 0, profile.numSegments() + 1 );

        } else {
            peak_detector( seg );

            auto info = peak_detector.getPeakInfo();
            info.setProtocol( fcn, profile.numSegments() + 1 );
            pInfo->addSegment( info );

            auto temp = std::make_shared< adcontrols::MassSpectrum >();
            peak_detector.getCentroidSpectrum( *temp );
            temp->setProtocol( fcn, profile.numSegments() + 1 );
            (*pCentroid) << std::move( temp );
        }
        ++fcn;
    }
    return { pCentroid, pInfo };
}
