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
                                                  , std::shared_ptr< adcontrols::MassSpectrum > refms_centroid )
    : proto_( proto )
    , mol_( mol )
    , refms_( refms )
    , refms_processed_( refms_centroid )
{
}

AutoTargetingCandidates::AutoTargetingCandidates( const AutoTargetingCandidates& t )
    : proto_( t.proto_ )
    , mol_( t.mol_ )
    , refms_( t.refms_  )
    , refms_processed_( t.refms_processed_ )
    , candidates_( t.candidates_ )
{
}

size_t
AutoTargetingCandidates::size() const
{
    return candidates_.size();
}

boost::optional< adcontrols::Targeting::Candidate >
AutoTargetingCandidates::operator []( size_t index ) const
{
    if ( candidates_.size() > index )
        return candidates_[ index ];
    return {};
}

void
AutoTargetingCandidates::set_candidates( const std::vector< adcontrols::Targeting::Candidate >& v )
{
    candidates_ = v;
}

void
AutoTargetingCandidates::set_mol( const adcontrols::moltable::value_type& t )
{
    mol_ = t;
}
