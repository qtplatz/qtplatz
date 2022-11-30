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

#include "mslocker.hpp"
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mschromatogrammethod.hpp>
#include <adcontrols/mslockmethod.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/segment_wrapper.hpp>

using namespace adprocessor;

msLocker::msLocker( const adcontrols::MSChromatogramMethod& cm, const adcontrols::ProcessMethod& pm )
{
    if ( auto lockm = pm.find< adcontrols::MSLockMethod >() ) {
        lockm_ = std::make_unique< adcontrols::MSLockMethod >( *lockm );
        if ( cm.lockmass() ) {
            std::copy_if( cm.molecules().data().begin(), cm.molecules().data().end()
                          , std::back_inserter( refs_ ), []( const auto& a ){ return a.flags() & adcontrols::moltable::isMSRef; } );
        }
    }
}

boost::optional< adcontrols::lockmass::mslock >
msLocker::operator()( const adcontrols::MassSpectrum& centroid )
{
    adcontrols::lockmass::mslock mslock;
    adcontrols::MSFinder find( lockm_->tolerance( lockm_->toleranceMethod() ), lockm_->algorithm(), lockm_->toleranceMethod() );
    for ( auto& ref : refs_ ) {
        if ( auto proto = ref.protocol() ) {
            if ( auto fms = centroid.findProtocol( *proto ) )  {
                size_t idx = find( *fms, ref.mass() );
                if ( idx != adcontrols::MSFinder::npos )
                    mslock << adcontrols::lockmass::reference( ref.formula(), ref.mass(), fms->mass( idx ), fms->time( idx ) );
            }
        } else {
            for ( auto& fms: adcontrols::segment_wrapper< const adcontrols::MassSpectrum >( centroid ) ) {
                size_t idx = find( fms, ref.mass() );
                if ( idx != adcontrols::MSFinder::npos )
                    mslock << adcontrols::lockmass::reference( ref.formula(), ref.mass(), fms.mass( idx ), fms.time( idx ) );
            }
        }
    }
    if ( mslock && mslock.fit() )
        return mslock;
    return boost::none;
}
