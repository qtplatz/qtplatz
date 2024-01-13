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

#include "findcompounds.hpp"
#include "quandatawriter.hpp"
#include "../plugins/dataproc/dataprocconstants.hpp"
#include <adcontrols/annotation.hpp>
#include <adcontrols/annotations.hpp>
#include <adcontrols/centroidmethod.hpp>
#include <adcontrols/centroidprocess.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/datasubscriber.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/histogram.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/lockmass.hpp>
#include <adcontrols/msfinder.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mslockmethod.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/peakresult.hpp>
#include <adcontrols/peaks.hpp>
#include <adcontrols/peak.hpp>
#include <adcontrols/processeddataset.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/quanmethod.hpp>
#include <adcontrols/quancompounds.hpp>
#include <adcontrols/quanresponse.hpp>
#include <adcontrols/quansample.hpp>
#include <adcontrols/quansequence.hpp>
#include <adcontrols/segment_wrapper.hpp>
#include <adcontrols/targeting.hpp>
#include <adcontrols/waveform_filter.hpp>
#include <adportable/debug.hpp>
#include <adprocessor/dataprocessor.hpp>


using namespace quan;

FindCompounds::FindCompounds( const adcontrols::QuanCompounds& cmpds
                              , const adcontrols::CentroidMethod& cm
                              , double tolerance ) : cm_( cm )
                                                   , compounds_( cmpds )
                                                   , tolerance_( tolerance ) {
}


bool
FindCompounds::doCentroid( std::shared_ptr< adprocessor::dataprocessor > dp
                           , std::shared_ptr< adcontrols::MassSpectrum > ms
                           , bool isCounting )
{
    size_t index = isCounting ? 0 : 1;

    centroid_[ index ] = std::make_shared< adcontrols::MassSpectrum >();
    pkinfo_[ index ]   = std::make_shared< adcontrols::MSPeakInfo >();
    profile_[ index ]  = ms;

    if ( dp->doCentroid( *pkinfo_[ index ], *centroid_[ index ], *ms, cm_ ) ) {
        return true;
    }
    return false;
}

namespace quan {
    struct findCompound {

        adcontrols::MSPeakInfo::iterator
        operator()( adcontrols::MSPeakInfo& xpkinfo
                    , const adcontrols::QuanCompound& compound
                    , double tolerance
                    , int protocol ) const {

            if ( compound.protocol() >= 0 && protocol != compound.protocol() )
                return xpkinfo.end();

            auto beg = std::lower_bound( xpkinfo.begin(), xpkinfo.end(), compound.mass() - tolerance, [](const auto& a, const double& m) { return a.mass() < m; });
            auto end = std::lower_bound( xpkinfo.begin(), xpkinfo.end(), compound.mass() + tolerance, [](const auto& a, const double& m) { return a.mass() < m; });

            // ADDEBUG() << "***** findCompound ***** " << compound.protocol() << ", " << protocol << ", " << (beg != xpkinfo.end() ? beg->mass() : -1.0);

            if ( beg != xpkinfo.end() && ( beg->mass() < compound.mass() + tolerance ) ) {

                auto pk = std::max_element( beg, end, [](const auto& a, const auto& b){ return a.area() < b.area(); } );
                pk->formula( compound.formula() ); // assign formula to peak
                pk->set_peak_index( std::distance( xpkinfo.begin(), pk ) );
                return pk;
            }
            return xpkinfo.end();
        }
    };
}

bool
FindCompounds::operator()( std::shared_ptr< adprocessor::dataprocessor > dp, bool isCounting )
{
    size_t index = isCounting ? 0 : 1;

    auto ms = profile_[ index ];
    auto centroid = centroid_[ index ];
    auto pkinfo = pkinfo_[ index ];

    if ( ! ( ms && centroid && pkinfo ) )
        return false;

    for ( auto& compound: compounds_ ) {

        if ( compound.isCounting() == isCounting ) {

            adcontrols::segment_wrapper< adcontrols::MassSpectrum > centroids( *centroid_[ index ] );

            int fcn(0);

            for ( auto& xpkinfo: adcontrols::segment_wrapper< adcontrols::MSPeakInfo >( *pkinfo ) ) {
                auto pk = findCompound()( xpkinfo, compound, tolerance_, fcn );
                if ( pk != xpkinfo.end() ) {
                    auto it = responses_.find( compound.uuid() );
                    if ( it == responses_.end() ) {
                        auto& resp = responses_[ compound.uuid() ];
                        resp.uuid_cmpd( compound.uuid() );
                        resp.uuid_cmpd_table( compounds_.uuid() );
                        resp.formula( compound.formula() );
                        resp.setPeakIndex( pk->peak_index() );
                        resp.setFcn( fcn );
                        resp.setMass( pk->mass() );

                        if ( isCounting ) { // histogram specific
                            double w = pk->centroid_right() - pk->centroid_left();
                            auto count = dp->countTimeCounts( adcontrols::segment_wrapper<const adcontrols::MassSpectrum>( *ms )[fcn], pk->mass() - w, pk->mass() + w );
                            resp.setIntensity( pk->area() ); // total counts
                            resp.setCountTimeCounts( count );
                        } else {
                            resp.setIntensity( xpkinfo.isAreaIntensity() ? pk->area() : pk->height() );
                            resp.setCountTimeCounts( xpkinfo.isAreaIntensity() ? pk->area() : pk->height() );  // pk->area() * trigCounts );
                        }

                        resp.setCountTriggers( adcontrols::segment_wrapper<const adcontrols::MassSpectrum>( *ms )[fcn].getMSProperty().numAverage() );
                        resp.setAmounts( 0 );
                        resp.set_tR( 0 );
                    } else {
                        ADDEBUG() << "duplicate peak identified within protocols for " << compound.formula();
                    }

                    using adcontrols::annotation;
                    centroids[fcn].addAnnotation({
                            compound.formula()
                            , pk->mass()
                            , pk->area()
                            , int( pk->peak_index() )
                            , 1000
                            , annotation::dataFormula });
                } else {
                    // ADDEBUG() << "*********** compound " << compound.formula() << ", " << compound.protocol() << " NOT FOUND at " << fcn;
                }
                ++fcn;
            }
        }
    }
    return true;
}


bool
FindCompounds::doMSLock( const adcontrols::MSLockMethod& m, bool isCounting )
{
    size_t index = isCounting ? 0 : 1;

    if ( centroid_[ index ] == nullptr )
        return false;

    // find reference peak by mass window
    auto mslock = std::make_shared< adcontrols::lockmass::mslock >();

    // TODO: consider how to handle segmented spectrum -- current impl is always process first
    adcontrols::MSFinder find( m.tolerance( m.toleranceMethod() ), m.algorithm(), m.toleranceMethod() );

    for ( auto& compound : compounds_ ) {
        if ( compound.isLKMSRef() ) {
            double exactMass = adcontrols::ChemicalFormula().getMonoIsotopicMass( compound.formula() );
            size_t idx = find( *centroid_[ index ], exactMass );
            if ( idx != adcontrols::MSFinder::npos ) {
                // add found peaks into mslock
                *mslock << adcontrols::lockmass::reference( compound.formula()
                                                            , exactMass
                                                            , centroid_[ index ]->mass( idx )
                                                            , centroid_[ index ]->time( idx ) );
            }
        }
    }
    mslock_ = mslock;

    if ( (*mslock_).fit() ) {
        for ( size_t idx = 0; idx < 2; ++idx ) {
            if ( centroid_[ idx ] )
                (*mslock_)( *centroid_[ idx ], true );
            if ( centroid_[ idx ] )
                (*mslock_)( *pkinfo_[ idx ], true );
            if ( profile_[ idx ] )
                (*mslock_)( *profile_[ idx ], true );
        }
        return true;
    }
    return false;
}

void
FindCompounds::write( std::shared_ptr< QuanDataWriter > writer
                      , const std::wstring& stem
                      , std::shared_ptr< const adcontrols::ProcessMethod > pm
                      , adcontrols::QuanSample& sample
                      , bool isCounting
                      , std::shared_ptr< adprocessor::dataprocessor > dp )
{
    size_t index = isCounting ? 0 : 1;

    auto name = stem + ( isCounting ? L" [C]" : L" [P]" );

    // save hinstogram on adfs filesystem
    if ( auto file = writer->write( *profile_[ index ], name ) ) {

        for ( auto& resp: responses_ )
            resp.second.setDataGuid( file.name() ); // dataGuid

        for ( const auto& resp: responses_ )
            sample << resp.second;

        auto att = writer->attach< adcontrols::MassSpectrum >( file, *centroid_[ index ], dataproc::Constants::F_CENTROID_SPECTRUM );
        writer->attach< adcontrols::ProcessMethod >( att, *pm, L"ProcessMethod" );
        writer->attach< adcontrols::MSPeakInfo >( file, *pkinfo_[ index ], dataproc::Constants::F_MSPEAK_INFO );
        writer->attach< adcontrols::QuanSample >( file, sample, dataproc::Constants::F_QUANSAMPLE );

        if ( isCounting ) {
            if ( auto sp = dp->massSpectrometer() ) {
                if ( auto profiled = adcontrols::histogram::make_profile( *profile_[ index ], *sp ) )
                    writer->attach< adcontrols::MassSpectrum >( file, *profiled, dataproc::Constants::F_PROFILED_HISTOGRAM );
            }
        }
    }
}
