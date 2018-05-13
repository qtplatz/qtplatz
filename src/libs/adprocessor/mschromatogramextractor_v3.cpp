/**************************************************************************
** Copyright (C) 2010-2018 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2018 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "mschromatogramextractor_v3.hpp"
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
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/moltable.hpp>
#include <adcontrols/mschromatogrammethod.hpp>
#include <adcontrols/msfinder.hpp>
#include <adcontrols/mslockmethod.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/waveform_filter.hpp>
#include <adcontrols/constants.hpp>
#include <adcontrols/datareader.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adportable/debug.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adportable/unique_ptr.hpp>
#include <adportable/utf.hpp>
#include <adutils/acquiredconf.hpp>
#include <boost/format.hpp>
#include <numeric>
#include <ratio>
#include <set>

namespace adprocessor {

    class v3::MSChromatogramExtractor::impl {
    public:
        impl( const adcontrols::LCMSDataset * raw ) : raw_( raw )
            {}

        void prepare_mslock( const adcontrols::MSChromatogramMethod&, const adcontrols::ProcessMethod& );
        void apply_mslock( std::shared_ptr< adcontrols::MassSpectrum >, const adcontrols::ProcessMethod&, adcontrols::lockmass::mslock& );
        void create_chromatograms( std::vector< std::shared_ptr< adcontrols::Chromatogram > >& vec
                                   , const adcontrols::MSChromatogramMethod& m );

        // [0]
        void append_to_chromatogram( size_t pos, const adcontrols::MassSpectrum& ms, const adcontrols::MSChromatogramMethod&, const std::string& );

        // [1]
        void append_to_chromatogram( size_t pos, const adcontrols::MassSpectrum& ms, const adcontrols::MSPeakInfo&, const std::string& );

        // [2]
        void append_to_chromatogram( size_t pos, const adcontrols::MassSpectrum& ms, adcontrols::hor_axis, const std::pair<double, double>& range, const std::string& );

        bool doMSLock( adcontrols::lockmass::mslock& mslock, const adcontrols::MassSpectrum& centroid, const adcontrols::MSLockMethod& m );
        bool doCentroid( adcontrols::MassSpectrum& centroid, const adcontrols::MassSpectrum& profile, const adcontrols::CentroidMethod& );

        std::vector< std::shared_ptr< mschromatogramextractor::xChromatogram > > results_; // vector<chromatogram>
        std::map< int, std::vector< std::shared_ptr< mschromatogramextractor::xChromatogram > > > xresults_; // fcn, vector<chromatogram>

        std::map< size_t, std::shared_ptr< adcontrols::MassSpectrum > > spectra_;
        const adcontrols::LCMSDataset * raw_;
        std::shared_ptr< adcontrols::MSLockMethod > lockm_;
        std::vector< std::pair< std::string, double > > msrefs_;
        std::shared_ptr< adcontrols::CentroidMethod > centroidMetod_;
    };

    struct protocol_finder {
        boost::optional< int > operator()( std::shared_ptr< const adcontrols::MassSpectrum > ms, const adcontrols::moltable::value_type& mol, double width ) {
            double lMass = mol.mass() - width / 2;
            double uMass = mol.mass() + width / 2;
            size_t nProto = ms->nProtocols();

            if ( mol.protocol() && ( nProto > mol.protocol().get() ) ) {

                auto& sp = adcontrols::segment_wrapper< const adcontrols::MassSpectrum >( *ms )[ mol.protocol().get() ];
                auto range = sp.getAcquisitionMassRange();
                if (  range.first < lMass && uMass < range.second )
                    return sp.protocolId();

            } else { // optional is none
                
                for ( auto& sp: adcontrols::segment_wrapper< const adcontrols::MassSpectrum >( *ms ) ) {
                    auto range = sp.getAcquisitionMassRange();
                    if (  range.first < lMass && uMass < range.second )
                        return sp.protocolId();
                }
                
            }
            return boost::none;
        }
    };

    struct cXtractor {
        double width;
        double lMass;
        double uMass;
        int32_t proto;
        std::shared_ptr< adcontrols::Chromatogram > pChr;
        
        cXtractor( double _w
                   , double _l
                   , double _u
                   , int32_t _p
                   , const std::wstring& desc = L"" ) : width( _w )
                                                      , lMass( _l )
                                                      , uMass( _u )
                                                      , proto( _p )
                                                      , pChr( std::make_shared< adcontrols::Chromatogram >() ) {
            pChr->addDescription( { L"Create", desc } );
        }
        
        inline void append( uint32_t pos, double time, double y ) {
            (*pChr) << std::make_pair( time, y );
        }
    };
}

using namespace adprocessor::v3;

MSChromatogramExtractor::~MSChromatogramExtractor()
{
    delete impl_;
}

MSChromatogramExtractor::MSChromatogramExtractor( const adcontrols::LCMSDataset * raw ) : impl_( new impl( raw ) )
{
}


bool
MSChromatogramExtractor::loadSpectra( const adcontrols::ProcessMethod * pm
                                      , std::shared_ptr< const adcontrols::DataReader > reader
                                      , int fcn
                                      , std::function<bool( size_t, size_t )> progress )
{
    size_t nSpectra = reader->size( fcn );
    
    if ( nSpectra == 0 )
        return false;
    
    progress( 0, nSpectra );
    
    impl_->results_.clear();

    const adcontrols::MSChromatogramMethod * cm = pm ? pm->find< adcontrols::MSChromatogramMethod >() : nullptr;
    bool doLock = cm ? cm->lockmass() : false;
    if ( doLock )
        impl_->prepare_mslock( *cm, *pm );

    adcontrols::lockmass::mslock mslock;    

    size_t n( 0 );

    for ( auto it = reader->begin( fcn ); it != reader->end(); ++it ) {
        
        auto ms = reader->readSpectrum( it );
#if 0
        // todo: filtering process should be a part of centroid process
        { // filter spectrum
            if ( auto fm = pm->find< adcontrols::CentroidMethod >() ) {
                if ( fm->noiseFilterMethod() == adcontrols::CentroidMethod::eDFTLowPassFilter )
                    adcontrols::waveform_filter::fft4c::lowpass_filter( *ms, fm->cutoffFreqHz() );
            }
            double base( 0 ), rms( 0 );
            const double * intens = ms->getIntensityArray();
            adportable::spectrum_processor::tic( uint32_t( ms->size() ), intens, base, rms );
            for ( size_t i = 0; i < ms->size(); ++i )
                ms->setIntensity( i, intens[ i ] - base );
        } // end filter
#endif
        
        if ( doLock )
            impl_->apply_mslock( ms, *pm, mslock );

#ifndef NDEBUG
        std::ostringstream o;
        for ( auto ref: mslock )
            o << ref.formula() << ", ";
        for ( auto a: mslock.coeffs() )
            o << a << ", ";
        ADDEBUG() << "mslock: proto=" << it->fcn() << "/" << fcn << " time: " << it->time_since_inject()
                  << " pos: " << it->pos() << ", " << it->rowid() << ", " << o.str();
#endif
        
        impl_->spectra_[ it->pos() ] = ms; // (:= pos sort order) keep mass locked spectral series
        
        if ( progress( ++n, nSpectra ) )
            return false;
    }
    return ! impl_->spectra_.empty();
}

///////////////////////////////////////////////////////////////////
////// [0] Create chromatograms by a list of molecules    /////////
///////////////////////////////////////////////////////////////////

bool
MSChromatogramExtractor::extract_by_mols( std::vector< std::shared_ptr< adcontrols::Chromatogram > >& vec
                                          , const adcontrols::ProcessMethod& pm
                                          , std::shared_ptr< const adcontrols::DataReader > reader
                                          , std::function<bool( size_t, size_t )> progress )
{
    vec.clear();
              
    if ( impl_->raw_->dataformat_version() <= 2 )
              return false;

    if ( auto cm = pm.find< adcontrols::MSChromatogramMethod >() ) {

        std::vector< cXtractor > temp;
        
        auto it = reader->begin( -1 );
        
        if ( auto sp = reader->readSpectrum( it ) ) {

            size_t cid(0);
            for ( auto& mol: cm->molecules().data() ) {
                if ( auto proto = protocol_finder()( sp, mol, cm->width_at_mass( mol.mass() ) ) )  {
                    if ( proto && mol.enable() ) {
                        double width = cm->width_at_mass( mol.mass() );
                        double lMass = mol.mass() - width / 2;
                        double uMass = mol.mass() + width / 2;
                        std::wstring desc = ( boost::wformat( L"%s %.4f (W:%.4gmDa) %s %d" )
                                              % adportable::utf::to_wstring( mol.formula() )
                                              % mol.mass()
                                              % ( width * 1000 )
                                              % adportable::utf::to_wstring( reader->display_name() )
                                              % proto.get() ).str();

                        temp.emplace_back( width, lMass, uMass, proto.get(), desc );

                        ADDEBUG() << "enable=" << mol.enable() << ", protocol=" << ( mol.protocol() ? mol.protocol().get() : -1 ) << ", " << mol.formula();
                    }
                }
            }
        }

        if ( temp.empty() )
            return false;

        if ( loadSpectra( &pm, reader, -1, progress ) ) {
            
            for ( auto& ms : impl_->spectra_ ) {
                for (auto& xc: temp ) {
                    auto& t = adcontrols::segment_wrapper< const adcontrols::MassSpectrum >( *ms.second )[ xc.proto ];
                    double time = t.getMSProperty().timeSinceInjection();
                    double y(0);
                    computeIntensity( y, t, adcontrols::hor_axis_mass, std::make_pair( xc.lMass, xc.uMass ) );
                    xc.append( ms.first /* pos */, time, y );
                }
            }
            
            std::pair< double, double > time_range =
                std::make_pair( impl_->spectra_.begin()->second->getMSProperty().timeSinceInjection()
                                , impl_->spectra_.rbegin()->second->getMSProperty().timeSinceInjection() );
            
            for ( auto& xc : temp ) {
                xc.pChr->minimumTime( time_range.first );
                xc.pChr->maximumTime( time_range.second );
                vec.emplace_back( std::move( xc.pChr ) );
            }
            return true;
        }
    }
    return false;
}

//////////
// [1] Create chromatograms from centroid result
bool
MSChromatogramExtractor::extract_by_peak_info( std::vector< std::shared_ptr< adcontrols::Chromatogram > >& vec
                                               , const adcontrols::ProcessMethod& pm
                                               , std::shared_ptr< const adcontrols::MSPeakInfo > pkinfo
                                               , std::shared_ptr< const adcontrols::DataReader > reader
                                               , std::function<bool( size_t, size_t )> progress )
{
    using namespace mschromatogramextractor;

    if ( impl_->raw_->dataformat_version() <= 2 )
        return false;

    if ( loadSpectra( &pm, reader, -1, progress ) ) {    
    
        uint32_t cid( 0 );

        for ( auto& ms : impl_->spectra_ ) {
            for ( const auto& info: adcontrols::segment_wrapper< const adcontrols::MSPeakInfo >( *pkinfo ) ) {
                if ( info.protocolId() == ms.second->protocolId() )
                    impl_->append_to_chromatogram( ms.first, *ms.second, info, reader->display_name() );
            }
        }
    
        std::pair< double, double > time_range =
            std::make_pair( impl_->spectra_.begin()->second->getMSProperty().timeSinceInjection()
                            , impl_->spectra_.rbegin()->second->getMSProperty().timeSinceInjection() );
        
        for ( auto& r : impl_->results_ ) {
            r->pChr_->minimumTime( time_range.first );
            r->pChr_->maximumTime( time_range.second );
            vec.push_back( r->pChr_ );
        }
        return true;
    }
    return false;
}

///////////////
// [2] Create chromatograms from selected range
bool
MSChromatogramExtractor::extract_by_axis_range( std::vector< std::shared_ptr< adcontrols::Chromatogram > >& vec
                                                , const adcontrols::ProcessMethod& pm
                                                , std::shared_ptr< const adcontrols::DataReader > reader
                                                , int fcn
                                                , adcontrols::hor_axis axis
                                                , const std::pair< double, double >& range
                                                , std::function<bool( size_t, size_t )> progress )
{
    if ( loadSpectra( &pm, reader, fcn, progress ) ) {

        for ( auto& ms : impl_->spectra_ ) {
            // [2]
            impl_->append_to_chromatogram( ms.first /*pos */, *ms.second, axis, range, reader->display_name() );
        }

        std::pair< double, double > time_range =
            std::make_pair( impl_->spectra_.begin()->second->getMSProperty().timeSinceInjection()
                          , impl_->spectra_.rbegin()->second->getMSProperty().timeSinceInjection() );
        
        for ( auto& r : impl_->results_ ) {
            r->pChr_->minimumTime( time_range.first );
            r->pChr_->maximumTime( time_range.second );
            vec.emplace_back( std::move( r->pChr_ ) );
        }
        return true;
    }

    return false;
}

// static
bool
MSChromatogramExtractor::computeIntensity( double& y, const adcontrols::MassSpectrum& ms, adcontrols::hor_axis axis, const std::pair< double, double >& range )
{
    if ( axis == adcontrols::hor_axis_mass ) {
        auto acqMrange = ms.getAcquisitionMassRange();
        const double lMass = range.first;
        const double uMass = range.second;

        if ( acqMrange.first < lMass && uMass < acqMrange.second ) {

            if ( ms.getMass( 0 ) <= lMass && uMass < ms.getMass( ms.size() - 1 ) ) {
                if ( ms.isCentroid() ) {
                    using mschromatogramextractor::accumulate;
                    y = accumulate<const double *>( ms.getMassArray(), ms.getIntensityArray(), ms.size() )( lMass, uMass );
                } else {
                    double base, rms;
                    double tic = adportable::spectrum_processor::tic( ms.size(), ms.getIntensityArray(), base, rms );
                    (void)tic;
                    adportable::spectrum_processor::areaFraction fraction;
                    adportable::spectrum_processor::getFraction( fraction, ms.getMassArray(), ms.size(), lMass, uMass );
                    y = adportable::spectrum_processor::area( fraction, base, ms.getIntensityArray(), ms.size() );
                }
            }
            return true;
        }
    } else {
        const double lTime = range.first;
        const double uTime = range.second;
        auto acqTrange = ms.getMSProperty().instTimeRange();
        if ( acqTrange.first < lTime && uTime < acqTrange.second ) {
            if ( ms.isCentroid() ) {
                using mschromatogramextractor::accumulate;
                y = accumulate<const double *>( ms.getTimeArray(), ms.getIntensityArray(), ms.size() )( lTime, uTime );
            } else {
                adportable::spectrum_processor::areaFraction fraction;
            
                fraction.lPos = ms.getIndexFromTime( lTime, false );
                fraction.uPos = ms.getIndexFromTime( uTime, false );
                {
                    double t0 = ms.getTime( fraction.lPos );
                    double t1 = ms.getTime( fraction.lPos + 1 );
                    assert( t0 < lTime && lTime < t1 );
                    fraction.lFrac = ( t1 - lTime ) / ( t1 - t0 );
                }
                {
                    double t0 = ms.getTime( fraction.uPos );
                    double t1 = ms.getTime( fraction.uPos + 1 );
                    assert( t0 < uTime && uTime < t1 );
                    fraction.uFrac = ( uTime - t0 ) / ( t1 - t0 );
                }
                double base(0), rms(0);
                adportable::spectrum_processor::tic( ms.size(), ms.getIntensityArray(), base, rms );
                y = adportable::spectrum_processor::area( fraction, base, ms.getIntensityArray(), ms.size() );
            }
            return true;
        }
    }

    return false;
}

// append chromatographic data point from a list of molecule [0]
void
MSChromatogramExtractor::impl::append_to_chromatogram( size_t pos
                                                       , const adcontrols::MassSpectrum& ms
                                                       , const adcontrols::MSChromatogramMethod& cm
                                                       , const std::string& display_name )
{
    using namespace mschromatogramextractor;
    
    adcontrols::segment_wrapper<const adcontrols::MassSpectrum> segments( ms );

    // const int protocol = ms.protocolId();        
    double time = ms.getMSProperty().timeSinceInjection();
    
    int cid(0); // chromatogram identifier in the given protocol
    for ( auto& m : cm.molecules().data() ) {
        
        if ( ! m.enable() )
            continue;

        int protocol( -1 );
        if ( auto p = m.protocol() )
            protocol = p.get();

        // ADDEBUG() << "protocol: " << protocol << ", " << m.formula();
        
        double width = cm.width_at_mass( m.mass() ); // cm.width( cm.widthMethod() );
        double lMass = m.mass() - width / 2;
        double uMass = m.mass() + width / 2;

        double y(0);
        if ( computeIntensity( y, ms, adcontrols::hor_axis_mass, std::make_pair( lMass, uMass ) ) ) {

            auto it = std::find_if( results_.begin(), results_.end(), [=]( std::shared_ptr<xChromatogram>& xc ) { return xc->fcn_ == protocol && xc->cid_ == cid; } );
            
            if ( it == results_.end() ) {
                results_.emplace_back( std::make_shared< xChromatogram >( m, width, protocol, cid, display_name ) );
                it = results_.end() - 1;
            }
            ( *it )->append( uint32_t( pos ), time, y );
        }
        ++cid;
    }
}

// [1]
void
MSChromatogramExtractor::impl::append_to_chromatogram( size_t pos
                                                       , const adcontrols::MassSpectrum& ms
                                                       , const adcontrols::MSPeakInfo& pkinfo
                                                       , const std::string& display_name )
{
    using namespace mschromatogramextractor;
    using adportable::utf;

    if ( ms.protocolId() != pkinfo.protocolId() )
        return;
    
    const int protocol = ms.protocolId();
    const double time = ms.getMSProperty().timeSinceInjection();
    
    uint32_t cid = 0;

    for ( auto& pk : pkinfo ) {
        
        double lMass = pk.mass() - pk.widthHH() / 2;
        double uMass = pk.mass() + pk.widthHH() / 2;

        double y(0);
        if ( computeIntensity( y, ms, adcontrols::hor_axis_mass, std::make_pair( lMass, uMass ) ) ) {

            auto it = std::find_if( results_.begin(), results_.end(), [=]( std::shared_ptr<xChromatogram>& xc ) { return xc->fcn_ == protocol && xc->cid_ == cid; } );
            
            if ( it == results_.end() ) {
                results_.emplace_back( std::make_shared< xChromatogram >( protocol, cid ) );
                it = results_.end() - 1;
                ( *it )->pChr_->addDescription(
                    adcontrols::description(
                        L"Create"
                        , ( boost::wformat( L"%s m/z %.4lf(W:%.4gmDa)_%d" )
                            % utf::to_wstring( display_name ) % pk.mass() % pk.widthHH() % protocol ).str() ) );
            }
            ( *it )->append( uint32_t( pos ), time, y );
        }
        ++cid;
    }
}    

// [2] Chromatogram from GUI selected m/z|time range
void
MSChromatogramExtractor::impl::append_to_chromatogram( size_t pos
                                                       , const adcontrols::MassSpectrum& ms
                                                       , adcontrols::hor_axis axis
                                                       , const std::pair< double, double >& range
                                                       , const std::string& display_name )
{
    using namespace mschromatogramextractor;
    using adportable::utf;
    
    const int protocol = ms.protocolId();        
    double time = ms.getMSProperty().timeSinceInjection();

    int cid(0); // chromatogram identifier in the given protocol
    double y(0);
    if ( computeIntensity( y, ms, axis, range ) ) {

        auto it = std::find_if( results_.begin(), results_.end(), [=]( std::shared_ptr<xChromatogram>& xc ) { return xc->fcn_ == protocol && xc->cid_ == cid; } );
            
        if ( it == results_.end() ) {
            results_.emplace_back( std::make_shared< xChromatogram >( protocol, cid ) );
            it = results_.end() - 1;
            double value_width = range.second - range.first;
            double value = range.first + value_width;
            if ( axis == adcontrols::hor_axis_mass ) {
                ( *it )->pChr_->addDescription( adcontrols::description(
                                                    L"Create"
                                                    , ( boost::wformat( L"%s m/z %.4lf(W:%.4gmDa)_%d" )
                                                        % utf::to_wstring( display_name ) % value % value_width % protocol ).str() ) );
            } else {
                ( *it )->pChr_->addDescription( adcontrols::description(
                                                    L"Create"
                                                    , ( boost::wformat( L"%s %.4lfus(W:%.4gns)_%d" )
                                                        % utf::to_wstring( display_name ) % (value*std::micro::den) % (value_width*std::nano::den) % protocol ).str() ) );
            }
        }
        ( *it )->append( uint32_t( pos ), time, y );
    }
}

void
MSChromatogramExtractor::impl::prepare_mslock( const adcontrols::MSChromatogramMethod& cm, const adcontrols::ProcessMethod& pm )
{
    msrefs_.clear();
    
    if ( cm.lockmass() ) {
        for ( auto& target : cm.molecules().data() ) {
            if ( target.flags() & adcontrols::moltable::isMSRef ) {
                double exactmass = adcontrols::ChemicalFormula().getMonoIsotopicMass( target.formula() );
                msrefs_.emplace_back( target.formula(), exactmass );
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
                                             , const adcontrols::ProcessMethod& pm
                                             , adcontrols::lockmass::mslock& mslock )
{
    if ( auto cm = pm.find< adcontrols::CentroidMethod >() ) {
        adcontrols::MassSpectrum centroid;
        doCentroid( centroid, *profile, *cm );

        doMSLock( mslock, centroid, *lockm_ );
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

    int mode = (-1);  // TODO: lock mass does not support rapid protocol 
    
    for ( auto& msref : msrefs_ ) {
        size_t proto = 0;
        for ( auto& fms: adcontrols::segment_wrapper< const adcontrols::MassSpectrum >( centroid ) ) {
            size_t idx = find( fms, msref.second );
            if ( idx != adcontrols::MSFinder::npos ) {
                if ( mode < 0 )
                    mode = fms.mode();
                if ( mode == fms.mode() ) {
                    mslock << adcontrols::lockmass::reference( msref.first, msref.second, fms.getMass( idx ), fms.getTime( idx ) );
                    // ADDEBUG() << "found ref: " << msref << "@ mode=" << mode << " proto=" << proto;
                } else {
                    ADDEBUG() << "found ref: " << msref << " but mode does not match.";
                }
            } else {
                // ADDEBUG() << "msref " << msref << " not found.";
            }
            ++proto;
        }
    }

    if ( mslock.fit() )
        return true;

    return false;
}
