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

#include "autotargeting.hpp"
#include "mslocker.hpp"
#include "dataprocessor.hpp"
#include <adcontrols/datareader.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mschromatogrammethod.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/targeting.hpp>
#include <adportable/debug.hpp>

using namespace adprocessor;

boost::optional< double >
AutoTargeting::find( int proto
                     , const adcontrols::moltable::value_type& mol
                     , const adcontrols::ProcessMethod& pm
                     , std::shared_ptr< const adcontrols::DataReader > reader
                     , std::function< void( const adcontrols::lockmass::mslock& )> callback )
{
    adcontrols::ProcessMethod localm;

    // cross check line 485, dataporocessworker.cpp in dataproc project
    auto cxm = pm.find< adcontrols::MSChromatogramMethod >();
    double pkw = cxm->peakWidthForChromatogram();

    if ( mol.tR() && *mol.tR() > 0 ) {

        double tR = *mol.tR();

        if ( auto cm = pm.find< adcontrols::CentroidMethod >() )
            localm.appendMethod( *cm );

        if ( auto tm = pm.find< adcontrols::TargetingMethod >() ) {
            auto it = std::find_if( tm->molecules().data().begin()
                                    , tm->molecules().data().end()
                                    , [&]( const auto& a ){ return a.protocol() == proto; } );
            if ( it != tm->molecules().data().end() ) {

                if ( auto ms = reader->coaddSpectrum( reader->findPos( tR - pkw/2.0 ), reader->findPos( tR + pkw/2.0 ) ) ) {

                    if ( auto res = dataprocessor::doCentroid( *ms, localm ) ) { // pkinfo, spectrum
                        if ( cxm->lockmass() ) {
                            msLocker locker( *cxm, pm );
                            if ( auto lock = locker( res->second ) ) {
                                (*lock)( res->second );  // caution -- res-first (pkinfo) not locked here.
                                callback( *lock );
                            }
                        }
                        auto targeting = adcontrols::Targeting( *tm );
                        if ( targeting.force_find( res->second, it->formula(), proto ) ) {
                            // --> debug
                            for ( const auto& c: targeting.candidates() )
                                ADDEBUG() << "candidata: " << c.formula << ", idx: " << c.idx << ", mass: " << c.mass << ", proto: " << c.fcn
                                          << ", error: " << ( c.mass - c.exact_mass ) * 1000 << "mDa";
                            // <--
                            return targeting.candidates().at(0).mass;
                        } else {
                            ADDEBUG() << "no target found";
                            return boost::none;
                        }
                    }
                }
            }
        }
    }
    return boost::none;
}
