/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "mschromatogramextractor_v2.hpp"
#include "mschromatogramextractor_accumulate.hpp"
#include "xchromatogram.hpp"
#include "centroidmethod.hpp"
#include "centroidprocess.hpp"
#include "chemicalformula.hpp"
#include "chromatogram.hpp"
#include "constants.hpp"
#include "description.hpp"
#include "descriptions.hpp"
#include "lcmsdataset.hpp"
#include "lockmass.hpp"
#include "massspectrum.hpp"
#include "moltable.hpp"
#include "mschromatogrammethod.hpp"
#include "msfinder.hpp"
#include "mslockmethod.hpp"
#include "msproperty.hpp"
#include "processmethod.hpp"
#include <adcontrols/constants.hpp>
#include <adcontrols/segment_wrapper.hpp>
#include <adportable/spectrum_processor.hpp>
#include "waveform_filter.hpp"
#include <boost/format.hpp>
#include <numeric>

namespace adprocessor {

    class v2::MSChromatogramExtractor::impl {
    public:
        impl( const adcontrols::LCMSDataset * raw ) : raw_( raw )
            {}

        void prepare_mslock( const adcontrols::MSChromatogramMethod&, const adcontrols::ProcessMethod& );
        void apply_mslock( std::shared_ptr< adcontrols::MassSpectrum >, const adcontrols::ProcessMethod&, adcontrols::lockmass::mslock& );
        void create_chromatograms( std::vector< std::shared_ptr< adcontrols::Chromatogram > >& vec
                                   , const adcontrols::MSChromatogramMethod& m );
        void append_to_chromatogram( size_t pos, const adcontrols::MassSpectrum& ms, const adcontrols::MSChromatogramMethod& );
        void append_to_chromatogram( size_t pos, const adcontrols::MassSpectrum& ms, const std::vector< std::pair<int, adcontrols::MSPeakInfoItem > >& ranges );
        size_t read_raw_spectrum( size_t pos, const adcontrols::LCMSDataset * raw, adcontrols::MassSpectrum& );

        bool doMSLock( adcontrols::lockmass::mslock& mslock, const adcontrols::MassSpectrum& centroid, const adcontrols::MSLockMethod& m );
        bool doCentroid( adcontrols::MassSpectrum& centroid, const adcontrols::MassSpectrum& profile, const adcontrols::CentroidMethod& );

        std::vector< std::shared_ptr< mschromatogramextractor::xChromatogram > > results_;

        // cache all spectra with lockmass
        std::map< size_t, std::shared_ptr< adcontrols::MassSpectrum > > spectra_;

        const adcontrols::LCMSDataset * raw_;
        std::shared_ptr< adcontrols::MSLockMethod > lockm_;
        std::vector< std::pair< std::string, double > > msrefs_;
        std::shared_ptr< adcontrols::CentroidMethod > centroidMetod_;
    };
}

using namespace adprocessor::v2;

MSChromatogramExtractor::~MSChromatogramExtractor()
{
    delete impl_;
}

MSChromatogramExtractor::MSChromatogramExtractor( const adcontrols::LCMSDataset * raw ) : impl_( new impl( raw ) )
{
}

bool
MSChromatogramExtractor::loadSpectra( const adcontrols::ProcessMethod * pm
                                      , std::function<bool( size_t, size_t )> progress )
{
    impl_->results_.clear();

    adcontrols::Chromatogram tic;
    size_t nSpectra(0);
    if ( impl_->raw_ && impl_->raw_->getTIC( 0, tic ) )
        nSpectra = tic.size();

    if ( nSpectra == 0 )
        return false;

    progress( 0, nSpectra );

    const adcontrols::MSChromatogramMethod * cm = pm ? pm->find< adcontrols::MSChromatogramMethod >() : nullptr;
    bool doLock = cm ? cm->lockmass() : false;
    if ( doLock )
        impl_->prepare_mslock( *cm, *pm );

    size_t pos = 0;
    size_t n( 0 );

    adcontrols::lockmass::mslock mslock;
    do {
        auto ms = std::make_shared< adcontrols::MassSpectrum >();

        if ( ( pos = impl_->read_raw_spectrum( pos, impl_->raw_, *ms ) ) ) {

            if ( cm->lockmass() ) {
                impl_->apply_mslock( ms, *pm, mslock );
            } else {
                adcontrols::lockmass::mslock lkms;
                if ( impl_->raw_->mslocker( lkms ) ) // if acquisition on-the-fly lock-mass
                    lkms( *ms );
            }
            impl_->spectra_[ pos - 1 ] = ms; // keep mass locked spectral series

            if ( progress( ++n, nSpectra ) )
                return false;
        }
    } while ( pos );

    return ! impl_->spectra_.empty();
}

bool
MSChromatogramExtractor::operator () ( std::vector< std::shared_ptr< adcontrols::Chromatogram > >& vec
                                       , const adcontrols::ProcessMethod& pm
                                       , std::function<bool( size_t, size_t )> progress )
{
    if ( const adcontrols::MSChromatogramMethod * cm = pm.find< adcontrols::MSChromatogramMethod >() ) {

        if ( loadSpectra( &pm, progress ) ) {

            auto prev = impl_->spectra_.begin()->second;
            for ( auto& ms : impl_->spectra_ ) {
                if ( ms.second->size() != prev->size() || ms.second->time( 0 ) != prev->time( 0 ) ) {
                    prev = ms.second;
                    // skip first spectrum after change condition; this elminates big artifact peak.
                } else {
                    impl_->append_to_chromatogram( ms.first, *ms.second, *cm );
                }
            }

            std::pair< double, double > time_range =
                std::make_pair( impl_->spectra_.begin()->second->getMSProperty().timeSinceInjection()
                                , impl_->spectra_.rbegin()->second->getMSProperty().timeSinceInjection() );

            for ( auto& r : impl_->results_ ) {
                r->pChr_->setMinimumTime( time_range.first );
                r->pChr_->setMaximumTime( time_range.second );
                vec.push_back( r->pChr_ );
            }

            return true;
        }
    }
    return false;
}

//////////
bool
MSChromatogramExtractor::operator () ( std::vector< std::shared_ptr< adcontrols::Chromatogram > >& vec
                                       , adcontrols::hor_axis axis
                                       , const std::vector< std::pair< int /* fcn */, adcontrols::MSPeakInfoItem > >& ranges
                                       , std::function<bool( size_t, size_t )> progress )
{
    using namespace mschromatogramextractor;

    if ( impl_->raw_->dataformat_version() >= 3 ) {

        assert( 0 );

    } else {

        adcontrols::Chromatogram tic;
        size_t nSpectra( 0 );
        if ( impl_->raw_ && impl_->raw_->getTIC( 0, tic ) )
            nSpectra = tic.size();

        if ( nSpectra == 0 )
            return false;

        progress( 0, nSpectra );

        impl_->results_.clear();

        //uint32_t target_index( 0 );
        for ( auto& range : ranges ) {
            //auto xc = std::make_shared< xChromatogram >( range, target_index++ );
            //impl_->results_.push_back( std::make_shared< xChromatogram >( range, target_index++ ) );
        }

        size_t pos = 0;
        size_t n( 0 );

        do {
            auto ms = std::make_shared< adcontrols::MassSpectrum >();

            if ( ( pos = impl_->read_raw_spectrum( pos, impl_->raw_, *ms ) ) ) {

                adcontrols::lockmass::mslock lkms;
                if ( impl_->raw_->mslocker( lkms ) ) // if acquisition on-the-fly lock-mass
                    lkms( *ms );

                impl_->spectra_ [ pos - 1 ] = ms; // keep mass locked spectral series

                if ( progress( ++n, nSpectra ) )
                    return false;
            }
        } while ( pos );

        for ( auto& ms : impl_->spectra_ )
            impl_->append_to_chromatogram( ms.first, *ms.second, ranges );

        std::pair< double, double > time_range =
            std::make_pair( impl_->spectra_.begin()->second->getMSProperty().timeSinceInjection()
                            , impl_->spectra_.rbegin()->second->getMSProperty().timeSinceInjection() );

        for ( auto& r : impl_->results_ ) {
            r->pChr_->setMinimumTime( time_range.first );
            r->pChr_->setMaximumTime( time_range.second );
            vec.push_back( r->pChr_ );
        }
        return true;
    }
    return false;
}

size_t
MSChromatogramExtractor::impl::read_raw_spectrum( size_t pos, const adcontrols::LCMSDataset * raw, adcontrols::MassSpectrum& ms )
{
    int idx, fcn, rep;
    while ( raw->index( pos, idx, fcn, rep ) && fcn != 0 )  // skip until 'fcn = 0' data
        ++pos;
    if ( raw->getSpectrum( -1, pos, ms ) ) // read all corresponding segments
        return pos + 1;
    return 0;
}

void
MSChromatogramExtractor::impl::append_to_chromatogram( size_t pos, const adcontrols::MassSpectrum& ms, const adcontrols::MSChromatogramMethod& cm )
{
    using namespace mschromatogramextractor;

    adcontrols::segment_wrapper<const adcontrols::MassSpectrum> segments( ms );

    double width = cm.width( cm.widthMethod() );
    (void)(width);

    uint32_t fcn = 0;

    for ( auto& fms: segments ) {

        double time = fms.getMSProperty().timeSinceInjection();

        uint32_t target_index = 0; //  index to the cm.targets();

        for ( auto& m : cm.molecules().data() ) {

            if ( ! m.enable() )
                continue;

            double width = cm.width_at_mass( m.mass() ); // cm.width( cm.widthMethod() );
            double lMass = m.mass() - width / 2;
            double uMass = m.mass() + width / 2;

            if ( fms.mass( 0 ) <= lMass && uMass < fms.mass( fms.size() - 1 ) ) {
                double y( 0 );
                if ( fms.isCentroid() ) {

                    y = accumulate<const double *>( fms.getMassArray(), fms.getIntensityArray(), fms.size() )( lMass, uMass );

                } else {

                    double base, rms;
                    double tic = adportable::spectrum_processor::tic( fms.size(), fms.getIntensityArray(), base, rms );
                    (void)tic;

                    adportable::spectrum_processor::areaFraction fraction;
                    adportable::spectrum_processor::getFraction( fraction, fms.getMassArray(), fms.size(), lMass, uMass );

                    y = adportable::spectrum_processor::area( fraction, base, fms.getIntensityArray(), fms.size() );

                }

                auto chro = std::find_if( results_.begin(), results_.end()
                                          , [=] ( std::shared_ptr<xChromatogram>& c ) {
                                              return c->fcn_ == fcn && c->cid_ == target_index;
                                          } );
                if ( chro == results_.end() ) {
                    results_.push_back( std::make_shared< xChromatogram >( m, width, fcn, target_index, std::string(), false ) );
                    chro = results_.end() - 1;
                }
                ( *chro )->append( uint32_t( pos ), time, y );
            }

            target_index++;
        }
        ++fcn;
    }

}

void
MSChromatogramExtractor::impl::append_to_chromatogram( size_t pos
                                                       , const adcontrols::MassSpectrum& ms
                                                       , const std::vector< std::pair<int, adcontrols::MSPeakInfoItem> >& ranges )
{
    using namespace mschromatogramextractor;

    adcontrols::segment_wrapper<const adcontrols::MassSpectrum> segments( ms );

    uint32_t target_index = 0;

    for ( auto& range : ranges ) {

        int rfcn = range.first;
        double lMass = range.second.mass() - range.second.widthHH() / 2; // std::get< 1 >( range );
        double uMass = range.second.mass() + range.second.widthHH() / 2; // std::get< 1 >( range );

        uint32_t fcn = 0;
        for ( auto& fms: segments ) {

            double time = fms.getMSProperty().timeSinceInjection();

            if ( ( fcn == rfcn ) &&
                 ( fms.mass( 0 ) <= lMass && uMass < fms.mass( fms.size() - 1 ) ) ) {

                double y( 0 );
                if ( fms.isCentroid() ) {

                    y = accumulate<const double *>( fms.getMassArray(), fms.getIntensityArray(), fms.size() )( lMass, uMass );

                } else {

                    double base, rms;
                    double tic = adportable::spectrum_processor::tic( fms.size(), fms.getIntensityArray(), base, rms );
                    (void)tic;
                    adportable::spectrum_processor::areaFraction fraction;
                    adportable::spectrum_processor::getFraction( fraction, fms.getMassArray(), fms.size(), lMass, uMass );
                    y = adportable::spectrum_processor::area( fraction, base, fms.getIntensityArray(), fms.size() );

                }

                auto chro = std::find_if( results_.begin(), results_.end()
                                          , [=] ( std::shared_ptr<xChromatogram>& c )
                                          {
                                              return c->fcn_ == fcn && c->cid_ == target_index;
                                          } );

                if ( chro != results_.end() )
                    ( *chro )->append( uint32_t( pos ), time, y );

            }
            ++fcn;
        }
        target_index++;
    }
}

void
MSChromatogramExtractor::impl::prepare_mslock( const adcontrols::MSChromatogramMethod& cm, const adcontrols::ProcessMethod& pm )
{
    msrefs_.clear();

    if ( cm.lockmass() ) {
        for ( auto& target : cm.molecules().data() ) {
            if ( target.flags() ) {
                double exactmass = adcontrols::ChemicalFormula().getMonoIsotopicMass( target.formula() );
                msrefs_.push_back( std::make_pair( target.formula(), exactmass ) );
            }
        }

        if ( auto lockm = pm.find< adcontrols::MSLockMethod >() ) {
            lockm_ = std::make_shared< adcontrols::MSLockMethod >( *lockm );
        } else {
            lockm_ = std::make_shared< adcontrols::MSLockMethod >();
        }
        lockm_->setAlgorithm( adcontrols::idFindLargest );
        lockm_->setTolerance( adcontrols::idToleranceDaltons, cm.tolerance() );
    }
}

void
MSChromatogramExtractor::impl::apply_mslock( std::shared_ptr< adcontrols::MassSpectrum > profile
                                             , const adcontrols::ProcessMethod& pm, adcontrols::lockmass::mslock& mslock )
{
    if ( auto cm = pm.find< adcontrols::CentroidMethod >() ) {

        adcontrols::MassSpectrum centroid;

        if ( cm->noiseFilterMethod() == adcontrols::CentroidMethod::eDFTLowPassFilter ) {
            adcontrols::MassSpectrum filtered;
            filtered.clone( *profile, true );
            for ( auto& ms : adcontrols::segment_wrapper<>( filtered ) ) {
                adcontrols::waveform_filter::fft4c::lowpass_filter( ms, cm->cutoffFreqHz() );
                double base( 0 ), rms( 0 );
                const double * intens = ms.getIntensityArray();
                adportable::spectrum_processor::tic( uint32_t( ms.size() ), intens, base, rms );
                for ( size_t i = 0; i < ms.size(); ++i )
                    ms.setIntensity( i, intens[ i ] - base );
            }
            // filtered.addDescription( adcontrols::description( L"process", dataproc::Constants::F_DFT_FILTERD ) );
            doCentroid( centroid, filtered, *cm );
        } else {
            doCentroid( centroid, *profile, *cm );
        }

        if ( centroid.size() > 0 ) {
            adcontrols::lockmass::mslock temp;
            if ( doMSLock( temp, centroid, *lockm_ ) )
                mslock = temp;
        }
    }

    if ( mslock )
        mslock( *profile );
}

bool
MSChromatogramExtractor::impl::doCentroid(adcontrols::MassSpectrum& centroid
                                          , const adcontrols::MassSpectrum& profile
                                          , const adcontrols::CentroidMethod& m )
{
    adcontrols::CentroidProcess peak_detector;
    bool result = false;

    centroid.clone( profile, false );

    if ( peak_detector( m, profile ) ) {
        result = peak_detector.getCentroidSpectrum( centroid );
        // pkInfo = peak_detector.getPeakInfo();
    }

    if ( profile.numSegments() > 0 ) {
        for ( size_t fcn = 0; fcn < profile.numSegments(); ++fcn ) {
            auto temp = std::make_shared< adcontrols::MassSpectrum >();
            result |= peak_detector( profile.getSegment( fcn ) );
            // pkInfo.addSegment( peak_detector.getPeakInfo() );
            peak_detector.getCentroidSpectrum( *temp );
            centroid <<  std::move( temp );
        }
    }
    return result;
}

bool
MSChromatogramExtractor::impl::doMSLock( adcontrols::lockmass::mslock& mslock
                                       , const adcontrols::MassSpectrum& centroid
                                       , const adcontrols::MSLockMethod& m )
{
    // TODO: consider how to handle segmented spectrum -- current impl is always process first
    adcontrols::MSFinder find( m.tolerance( m.toleranceMethod() ), m.algorithm(), m.toleranceMethod() );

    for ( auto& msref : msrefs_ ) {
        size_t idx = find( centroid, msref.second );
        if ( idx != adcontrols::MSFinder::npos )
            mslock << adcontrols::lockmass::reference( msref.first, msref.second, centroid.mass( idx ), centroid.time( idx ) );
    }

    if ( mslock.fit() ) {
        // mslock( centroid, true );
        return true;
    }
    return false;
}
