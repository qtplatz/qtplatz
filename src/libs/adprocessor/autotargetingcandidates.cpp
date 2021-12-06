/**************************************************************************
** Copyright (C) 2010-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2022 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "autotargetingcandidates.hpp"
#include <adcontrols/moltable.hpp>
#include <adcontrols/targeting.hpp>
#include <boost/optional.hpp>
#include <memory>

namespace adcontrols {
    class MassSpectrum;
}

using namespace adprocessor;

AutoTargetingCandidates::AutoTargetingCandidates() : proto_( 0 )
{
}

AutoTargetingCandidates::AutoTargetingCandidates( int proto
                                                  , const adcontrols::moltable::value_type& mol
                                                  , std::shared_ptr< adcontrols::MassSpectrum > refms
                                                  , std::shared_ptr< adcontrols::MassSpectrum > refms_centroid
                                                  , std::shared_ptr< adcontrols::MSPeakInfo > pkinfo )

    : proto_( proto )
    , mol_( mol )
    , refms_( refms )
    , refms_processed_( refms_centroid )
    , refms_pkinfo_( pkinfo )
{
}

AutoTargetingCandidates::AutoTargetingCandidates( const AutoTargetingCandidates& t )
    : proto_( t.proto_ )
    , mol_( t.mol_ )
    , refms_( t.refms_  )
    , refms_processed_( t.refms_processed_ )
    , refms_pkinfo_( t.refms_pkinfo_ )
    , targeting_( t.targeting_ )
{
}

size_t
AutoTargetingCandidates::size() const
{
    return targeting_ ? targeting_->candidates().size() : 0;
}

boost::optional< adcontrols::Targeting::Candidate >
AutoTargetingCandidates::operator []( size_t index ) const
{
    if ( targeting_ && targeting_->candidates().size() > index )
        return targeting_->candidates().at( index );
    return {};
}

void
AutoTargetingCandidates::set_mol( const adcontrols::moltable::value_type& t )
{
    mol_ = t;
}

void
AutoTargetingCandidates::set_mol( adcontrols::moltable::value_type&& t )
{
    mol_ = t;
}

void
AutoTargetingCandidates::set_targeting( std::shared_ptr< adcontrols::Targeting > t )
{
    targeting_ = t;
}

void
AutoTargetingCandidates::set_targeting( std::shared_ptr< adcontrols::Targeting >&& t )
{
    targeting_ = t;
}
