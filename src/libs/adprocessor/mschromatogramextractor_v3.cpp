/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include "dataprocessor.hpp"
#include "description.hpp"
#include "descriptions.hpp"
#include "lcmsdataset.hpp"
#include "lockmass.hpp"
#include <adacquire/constants.hpp>
#include <adcontrols/constants.hpp>
#include <adcontrols/datareader.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/moltable.hpp>
#include <adcontrols/mschromatogrammethod.hpp>
#include <adcontrols/msfinder.hpp>
#include <adcontrols/mslockmethod.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/quanresponsemethod.hpp>
#include <adcontrols/targeting.hpp>
#include <adcontrols/waveform_filter.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/date_time.hpp>
#include <adportable/debug.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adportable/unique_ptr.hpp>
#include <adportable/utf.hpp>
#include <adutils/acquiredconf.hpp>
#include <boost/format.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <algorithm>
#include <numeric>
#include <ratio>
#include <regex>
#include <set>

namespace adprocessor {

    struct msLocker;

    class v3::MSChromatogramExtractor::impl {
    public:
        impl( const adcontrols::LCMSDataset * raw ) : raw_( raw )
            {}

        // void prepare_mslock( const adcontrols::MSChromatogramMethod&, const adcontrols::ProcessMethod& );
        bool apply_mslock( std::shared_ptr< adcontrols::MassSpectrum >, const adcontrols::ProcessMethod&, adcontrols::lockmass::mslock& );
        void create_chromatograms( std::vector< std::shared_ptr< adcontrols::Chromatogram > >& vec
                                   , const adcontrols::MSChromatogramMethod& m );

        // [0]
        void append_to_chromatogram( size_t pos, const adcontrols::MassSpectrum& ms, const adcontrols::MSChromatogramMethod&, const std::string& );

        // [1]
        void append_to_chromatogram( size_t pos, const adcontrols::MassSpectrum& ms, const adcontrols::MSPeakInfo&, const std::string& );

        // [2]
        void append_to_chromatogram( size_t pos, const adcontrols::MassSpectrum& ms, adcontrols::hor_axis, const std::pair<double, double>& range, const std::string& );

        // bool doMSLock( adcontrols::lockmass::mslock& mslock, const adcontrols::MassSpectrum& centroid );
        bool doCentroid( adcontrols::MassSpectrum& centroid, const adcontrols::MassSpectrum& profile, const adcontrols::CentroidMethod& );

        std::vector< std::shared_ptr< mschromatogramextractor::xChromatogram > > results_; // vector<chromatogram>
        std::map< int, std::vector< std::shared_ptr< mschromatogramextractor::xChromatogram > > > xresults_; // fcn, vector<chromatogram>

        std::map< size_t, std::shared_ptr< adcontrols::MassSpectrum > > spectra_;
        const adcontrols::LCMSDataset * raw_;
        std::shared_ptr< adcontrols::CentroidMethod > centroidMetod_;
        //
        std::unique_ptr< msLocker > msLocker_;
        adcontrols::lockmass::mslock mslock_; // mslock at auto-targeting
        std::vector< std::pair< int64_t, std::array< double, 2 > > > lkms_;  // time, coeffs
    };

    struct protocol_finder {
        boost::optional< int > operator()( std::shared_ptr< const adcontrols::MassSpectrum > ms, const adcontrols::moltable::value_type& mol, double width ) {
            double lMass = mol.mass() - width / 2;
            double uMass = mol.mass() + width / 2;
            size_t nProto = ms->nProtocols();

            if ( mol.protocol() && ( nProto > mol.protocol().get() ) ) {

                auto& sp = adcontrols::segment_wrapper< const adcontrols::MassSpectrum >( *ms )[ mol.protocol().get() ];
                auto range = sp.getMSProperty().instMassRange(); // don't use getAcquisitionmassrange() that is full acq. range
                if (  range.first < lMass && uMass < range.second )
                    return sp.protocolId();

            } else { // optional is none

                for ( auto& sp: adcontrols::segment_wrapper< const adcontrols::MassSpectrum >( *ms ) ) {
                    auto range = sp.getMSProperty().instMassRange(); // don't use getAcquisitionmassrange() that is full acq. range
                    //ADDEBUG() << "find poto: " << range << ", " << mol.formula() << " proto=" << sp.protocolId();
                    if (  range.first < lMass && uMass < range.second ) {
                        //ADDEBUG() << "\tfound: " << sp.protocolId();
                        return sp.protocolId();
                    }
                }

            }
            return boost::none;
        }
    };

    struct cXtractor {
        double mass;
        double width;
        double lMass;
        double uMass;
        int32_t proto;
        std::shared_ptr< adcontrols::Chromatogram > pChr;

        cXtractor( double _m
                   , double _w
                   , double _l
                   , double _u
                   , int32_t _p
                   , const std::wstring& desc = L"" ) : mass( _m )
                                                      , width( _w )
                                                      , lMass( _l )
                                                      , uMass( _u )
                                                      , proto( _p )
                                                      , pChr( std::make_shared< adcontrols::Chromatogram >() ) {
            pChr->addDescription( { L"Create", desc } );
            pChr->setProtocol( _p );
        }

        inline void append( uint32_t pos, double time, double y ) {
            (*pChr) << std::make_pair( time, y );
        }
        inline void append( uint32_t pos, double time, double y, double tof, double mass ) {
            (*pChr) << std::make_tuple( time, y, tof, mass );
        }
    };

    struct msLocker {

        std::vector< adcontrols::moltable::value_type > refs_;
        adcontrols::MSLockMethod lockm_;
        msLocker( const adcontrols::MSChromatogramMethod& cm, const adcontrols::ProcessMethod& pm ) {
            if ( auto lockm = pm.find< adcontrols::MSLockMethod >() ) {
                lockm_ = *lockm;
                if ( cm.lockmass() ) {
                    std::copy_if( cm.molecules().data().begin(), cm.molecules().data().end()
                                  , std::back_inserter( refs_ ), []( const auto& a ){ return a.flags() & adcontrols::moltable::isMSRef; } );
                }
            }
        }

        boost::optional< adcontrols::lockmass::mslock >
        operator()( const adcontrols::MassSpectrum& centroid ) {
            adcontrols::lockmass::mslock mslock;
            adcontrols::MSFinder find( lockm_.tolerance( lockm_.toleranceMethod() ), lockm_.algorithm(), lockm_.toleranceMethod() );
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
    };


    struct AutoTargeting {
        adcontrols::ProcessMethod localm;
        boost::optional< double > find( int proto
                                        , const adcontrols::moltable::value_type& mol
                                        , const adcontrols::ProcessMethod& pm
                                        , std::shared_ptr< const adcontrols::DataReader > reader
                                        , std::function< void( const adcontrols::lockmass::mslock& )> callback ) {

            // cross check line 485, dataporocessworker.cpp in dataproc project
            auto cxm = pm.find< adcontrols::MSChromatogramMethod >();
            double pkw = cxm->peakWidthForChromatogram();

            if ( mol.tR() && *mol.tR() > 0 ) {

                double tR = *mol.tR();

                if ( auto cm = pm.find< adcontrols::CentroidMethod >() )
                    localm.appendMethod( *cm );

                if ( auto tm = pm.find< adcontrols::TargetingMethod >() ) {
                    auto it = std::find_if( tm->molecules().data().begin(), tm->molecules().data().end()
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

std::shared_ptr< const adcontrols::MassSpectrum >
MSChromatogramExtractor::getMassSpectrum( double tR ) const
{
    // depend on the timing of this call, either waveform or histogram will be returned
    auto it = std::lower_bound( impl_->spectra_.begin(), impl_->spectra_.end(), tR
                                , [&]( const auto& pair, double t ){ return pair.second->getMSProperty().timeSinceInjection() < t; });
    if ( it == impl_->spectra_.end() )
        return nullptr;
    return it->second;
}

bool
MSChromatogramExtractor::loadSpectra( const adcontrols::ProcessMethod * pm
                                      , std::shared_ptr< const adcontrols::DataReader > reader
                                      , int fcn
                                      , std::function<bool( size_t, size_t )> progress )
{
    const size_t nSpectra = reader->size( fcn );
    const bool isProfile = ( reader->objtext().find( "waveform" ) != std::string::npos ) ||
        std::regex_search( reader->objtext(), std::regex( "^[1-9]\\.u5303a\\.ms-cheminfo.com" ) );
    (void)isProfile;

    if ( nSpectra == 0 )
        return false;

    progress( 0, nSpectra );

    impl_->results_.clear();
    impl_->msLocker_.reset();
    const adcontrols::MSChromatogramMethod * cm = pm ? pm->find< adcontrols::MSChromatogramMethod >() : nullptr;
    if ( cm->lockmass() )
        impl_->msLocker_ = std::make_unique< msLocker > ( *cm, *pm );

    adcontrols::lockmass::mslock mslock;

    size_t n( 0 );

    for ( auto it = reader->begin( fcn ); it != reader->end(); ++it ) {

        auto ms = reader->readSpectrum( it );

        if ( cm->lockmass() ) {
            if ( impl_->apply_mslock( ms, *pm, mslock ) ) {
                std::array< double, 2 > coeffs;
                if ( mslock.coeffs().size() == 1 )
                    coeffs = {{ 0.0, mslock.coeffs()[ 0 ] }};
                else
                    coeffs = {{ mslock.coeffs()[ 0 ], mslock.coeffs()[ 1 ] }};
                impl_->lkms_.emplace_back( it->epoch_time(), coeffs );
            }
        }

#if ! defined NDEBUG && 0
        std::ostringstream o;
        for ( auto ref: mslock )
            o << ref.formula() << ", ";
        for ( auto a: mslock.coeffs() )
            o << a << ", ";
        ADDEBUG() << "mslock: " << " proto=" << it->fcn() << "/" << fcn << " time: " << it->time_since_inject()
                  << " pos: " << it->pos() << ", " << it->rowid() << ", " << o.str();
#endif
        impl_->spectra_[ it->pos() ] = ms; // (:= pos sort order) keep mass locked spectral series

        if ( progress( ++n, nSpectra ) )
            return false;
    }
    return ! impl_->spectra_.empty();
}

std::chrono::time_point< std::chrono::system_clock, std::chrono::nanoseconds >
MSChromatogramExtractor::time_of_injection() const
{
    if ( auto db = impl_->raw_->db() ) {
        adfs::stmt sql( *db );
        sql.prepare( "SELECT epoch_time,events FROM AcquiredData WHERE events >= ? ORDER BY epoch_time" );
        sql.bind( 1 ) = uint32_t( adacquire::SignalObserver::wkEvent_INJECT );  // most likely also set wkEvent_AcqInProgress bit
        if ( sql.step() == adfs::sqlite_row ) {
            if ( sql.get_column_value< int64_t >(1) & adacquire::SignalObserver::wkEvent_INJECT )
                return std::chrono::system_clock::time_point() + std::chrono::nanoseconds( sql.get_column_value< int64_t >(0) );
        } else
            ADDEBUG() << "SQL Error : " << sql.errmsg();
    }
    return {}; // return epoch
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
    impl_->spectra_.clear();
    impl_->mslock_ = {};

    if ( impl_->raw_->dataformat_version() <= 2 )
        return false;

    ADDEBUG() << "extract_by_mols";

    std::unique_ptr< adcontrols::CentroidProcess > peak_detector;
    std::unique_ptr< adcontrols::MSFinder > msfinder;

    bool areaIntensity( true );
    if ( auto qrm = pm.find< adcontrols::QuanResponseMethod >() ) {
        if ( qrm->intensityMethod() == adcontrols::QuanResponseMethod::idCentroid ) {
            if ( auto cm = pm.find< adcontrols::CentroidMethod >() ) {
                areaIntensity = cm->centroidAreaIntensity();
                peak_detector = std::make_unique< adcontrols::CentroidProcess >( *cm );
                msfinder = std::make_unique< adcontrols::MSFinder >( qrm->width(), qrm->findAlgorithm(), adcontrols::idToleranceDaltons );
            }
        }
    }

    if ( auto cm = pm.find< adcontrols::MSChromatogramMethod >() ) {

        std::vector< cXtractor > temp;
        auto it = reader->begin( -1 );

        if ( auto sp = reader->readSpectrum( it ) ) {

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

                        if ( cm->enableAutoTargeting() ) {
                            if ( auto mass = AutoTargeting().find( *proto, mol, pm, reader
                                                                   , [&]( const adcontrols::lockmass::mslock& lock ){ impl_->mslock_ = lock; } ) ) {
                                lMass = *mass - width / 2;
                                uMass = *mass + width / 2;
                                desc = ( boost::wformat( L"%s %.4f AT (W:%.4gmDa) %s %d" )
                                         % adportable::utf::to_wstring( mol.formula() )
                                         % *mass
                                         % ( width * 1000 )
                                         % adportable::utf::to_wstring( reader->display_name() )
                                         % proto.get() ).str();
                            }
                        }

                        auto& t = adcontrols::segment_wrapper< const adcontrols::MassSpectrum >( *sp )[ proto.get() ];
                        double tof = t.getTime( t.getIndexFromMass( mol.mass() ) );

                        auto time_of_injection = this->time_of_injection();

                        boost::property_tree::ptree pt;
                        pt.put( "generator.time_of_injection", adportable::date_time::to_iso< std::chrono::nanoseconds >( time_of_injection ) );
                        if ( auto molid = mol.property< boost::uuids::uuid >( "molid" ) )
                            pt.put( "generator.extract_by_mols.molid", molid.get() );
                        pt.put( "generator.extract_by_mols.wform_type", (sp->isCentroid() ? "centroid" : "profile") );
                        pt.put( "generator.extract_by_mols.moltable.protocol", proto.get() );
                        pt.put( "generator.extract_by_mols.moltable.mass", mol.mass() );
                        pt.put( "generator.extract_by_mols.moltable.width", width );
                        pt.put( "generator.extract_by_mols.moltable.formula", mol.formula() );
                        if ( cm->lockmass() )
                            pt.put( "generator.extract_by_mols.moltable.msref", mol.isMSRef() );
                        pt.put( "generator.extract_by_mols.tof", tof );

                        if ( peak_detector )
                            pt.put( "generator.extract_by_mols.centroid", ( areaIntensity ? "area" : "height" ) );

                        temp.emplace_back( mol.mass(), width, lMass, uMass, (proto ? proto.get() : -1), desc );
                        temp.back().pChr->setGeneratorProperty( pt );
                        temp.back().pChr->set_time_of_injection( std::move( time_of_injection ) );
                        if ( sp->isHistogram() ) {
                            temp.back().pChr->setAxisLabel( adcontrols::plot::yAxis, "Counts" );
                            temp.back().pChr->setAxisUnit( adcontrols::plot::Counts );
                        } else {
                            temp.back().pChr->setAxisLabel( adcontrols::plot::yAxis, areaIntensity ? "Intensity (area)" : "Intensity" );
                            temp.back().pChr->setAxisUnit( adcontrols::plot::Volts, 1000 ); // mV
                        }
#if !defined NDEBUG
                        ADDEBUG() << pt;
#endif
                    }
                }
            }
        }

        if ( temp.empty() )
            return false;

        // Generate chromatograms
        if ( loadSpectra( &pm, reader, -1, progress ) ) {

            for ( auto& ms : impl_->spectra_ ) {
                for (auto& xc: temp ) {
                    try {
                        auto& t = adcontrols::segment_wrapper< const adcontrols::MassSpectrum >( *ms.second )[ xc.proto ];
                        double time = t.getMSProperty().timeSinceInjection();
                        auto y = computeIntensity( t, adcontrols::hor_axis_mass, std::make_pair( xc.lMass, xc.uMass ) );
                        xc.append( ms.first, time, y ? y.get() : 0 );
                    } catch ( std::out_of_range& ex ) {
                        ADDEBUG() << ex.what() << "\t-- skip this data point"; // ignore and continue (no chromatogram data added)
                    } catch ( std::exception& ex ) {
                        ADDEBUG() << ex.what();
                        return false;
                    }
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

    ADDEBUG() << "extract_by_peak_info";

    if ( loadSpectra( &pm, reader, -1, progress ) ) {

        for ( auto& ms : impl_->spectra_ ) {
            for ( const auto& info: adcontrols::segment_wrapper< const adcontrols::MSPeakInfo >( *pkinfo ) ) {
                if ( info.protocolId() == ms.second->protocolId() ) {
                    impl_->append_to_chromatogram( ms.first, *ms.second, info, reader->display_name() );
                }
            }
        }

        std::pair< double, double > time_range =
            std::make_pair( impl_->spectra_.begin()->second->getMSProperty().timeSinceInjection()
                            , impl_->spectra_.rbegin()->second->getMSProperty().timeSinceInjection() );

        for ( auto& r : impl_->results_ ) {
            r->pChr_->minimumTime( time_range.first );
            r->pChr_->maximumTime( time_range.second );
            r->pChr_->setAxisLabel( adcontrols::plot::yAxis, r->isCounting_ ? "Counts" : "Intensity" );
            r->pChr_->setAxisUnit( r->isCounting_ ? adcontrols::plot::Counts : adcontrols::plot::Arbitrary );
            vec.emplace_back( std::move( r->pChr_ ) );
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
    ADDEBUG() << "extract_by_axis_range";

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
            r->pChr_->setAxisLabel( adcontrols::plot::yAxis, r->isCounting_ ? "Counts" : "Intensity" );
            r->pChr_->setAxisUnit( r->isCounting_ ? adcontrols::plot::Counts : adcontrols::plot::Arbitrary );
            vec.emplace_back( std::move( r->pChr_ ) );
        }
        return true;
    }

    return false;
}

// [3] Chromatograms from targeting result json
bool
MSChromatogramExtractor::extract_by_json( std::vector< std::shared_ptr< adcontrols::Chromatogram > >& vec
                                          , const adcontrols::ProcessMethod& pm
                                          , std::shared_ptr< const adcontrols::DataReader > reader
                                          , const std::string& json
                                          , double width
                                          , adcontrols::hor_axis axis
                                          , std::function<bool( size_t, size_t )> progress )
{
    boost::property_tree::ptree pt;

    {
        std::istringstream in( json );
        boost::property_tree::read_json( in, pt );
    }
    const char * const wkey = (axis == adcontrols::hor_axis_mass) ? "mass" : "time";

    std::vector< std::pair< std::pair< double, double >, int > > list;  // pair< range >, fcn
    std::vector< std::string > mols;

    if ( auto formulae = pt.get_child_optional( "formulae" ) ) {
        for ( auto& formula: formulae.get() ) {
            if ( auto selected = formula.second.get_optional< bool >( "selected" ) ) {
                if ( selected.get() ) {
                    int proto = 0;
                    if ( auto pno = formula.second.get_optional< int >( "protocol" ) ) {
                        proto = pno.get();
                    } else if ( auto pno = formula.second.get_optional< int >( "proto" ) ) {
                        proto = pno.get();
                    }
                    if ( auto centre = formula.second.get_optional< double >( wkey ) ) {
                        list.emplace_back( std::make_pair( centre.get() - width / 2, centre.get() + width / 2 ), proto );
                        auto mol = formula.second.get_optional< std::string >( "formula" );
                        mols.emplace_back( mol ? mol.get() : "no-formula" );
                    }
                }
            }

            if ( auto children = formula.second.get_child_optional("children") ) {
                for ( auto child: children.get() ) {
                    if ( auto selected = child.second.get_optional< bool >( "selected" ) ) {
                        int proto = 0;
                        if ( auto pno = child.second.get_optional< int > ( "protocol" ) ) {
                            proto = pno.get();
                        } else if ( auto pno = formula.second.get_optional< int >( "proto" ) ) {
                            proto = pno.get();
                        }
                        if ( auto centre = child.second.get_optional< double >( wkey ) ) {
                            list.emplace_back( std::make_pair( centre.get() - width / 2, centre.get() + width / 2 ), proto );
                            auto mol = formula.second.get_optional< std::string >( "formula" );
                            mols.emplace_back( mol ? mol.get() : "no-formula" );
                        }
                    }
                }
            }
        }
    }

    if ( loadSpectra( &pm, reader, -1, progress ) ) {

        const bool isCounting = std::regex_search( reader->objtext(), std::regex( "^pkd\\.[1-9]\\.u5303a\\.ms-cheminfo.com" ) ); // pkd is counting

        ADDEBUG() << "########## isCounting: " << isCounting;

        auto fmt = ( axis == adcontrols::hor_axis_mass ) ? boost::format( "%s %.1f(W:%.1fmDa) %s p%d" ) : boost::format( "%s %.4lfus(W:%.1ns) %s p%d" );

        for ( size_t idx = 0; idx < list.size(); ++idx ) {
            auto res = std::make_shared< mschromatogramextractor::xChromatogram >( list[ idx ].second, idx, isCounting );
            int protocol = list[ idx ].second;
            double width = list[ idx ].first.second - list[ idx ].first.first;
            double centre = list[ idx ].first.first + width / 2.0;
            const std::string& formula = mols[ idx ];
            if ( axis == adcontrols::hor_axis_mass ) {
                width *= 1000;             // --> mDa
            } else {
                centre *= std::micro::den; // --> us
                width *= std::nano::den;   // --> ns
            }
            res->pChr_->addDescription(
                adcontrols::description( { "Create", ( fmt % formula % centre % width % reader->display_name() % protocol ).str() } ) );
            res->pChr_->setIsCounting( res->isCounting_ );
            impl_->results_.emplace_back( res );
        }

        // compute each point on the chromatogram
        for ( auto& ms : impl_->spectra_ ) {
            size_t cid(0);
            for ( const auto& item: list ) {
                const int proto = item.second;
                const uint32_t pos = ms.first;
                if ( const auto pms = ms.second->findProtocol( proto ) ) {
                    double time = pms->getMSProperty().timeSinceInjection();
                    auto y = computeIntensity( *pms, axis, item.first );
                    impl_->results_[ cid ]->append( pos, time, y ? y.get() : 0 );
                } else {
                    double time = ms.second->getMSProperty().timeSinceInjection();
                    impl_->results_[ cid ]->append( pos, time, 0 ); // assin zero if no data found.
                }
                ++cid;
            }
        }

        std::pair< double, double > time_range =
            std::make_pair( impl_->spectra_.begin()->second->getMSProperty().timeSinceInjection()
                            , impl_->spectra_.rbegin()->second->getMSProperty().timeSinceInjection() );

        for ( auto& r : impl_->results_ ) {
            r->pChr_->minimumTime( time_range.first );
            r->pChr_->maximumTime( time_range.second );
            if ( r->isCounting_ ) {
                r->pChr_->setAxisLabel( adcontrols::plot::yAxis, "Counts" );
                r->pChr_->setAxisUnit( adcontrols::plot::Counts );
            } else {
                r->pChr_->setAxisLabel( adcontrols::plot::yAxis, "Intensity" );
                r->pChr_->setAxisUnit( adcontrols::plot::Volts, 1000 );
            }
            vec.emplace_back( std::move( r->pChr_ ) );
        }
        return true;
    }

    return false;
}

boost::optional< double >
MSChromatogramExtractor::computeIntensity( const adcontrols::MassSpectrum& ms, adcontrols::hor_axis axis, const std::pair< double, double >& range )
{
    double y(0);
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
            return y;
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
            return y;
        }
    }
    return boost::none;
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

        if ( auto y = computeIntensity( ms, adcontrols::hor_axis_mass, std::make_pair( lMass, uMass ) ) ) {

            auto it = std::find_if( results_.begin(), results_.end(), [=]( std::shared_ptr<xChromatogram>& xc ) { return xc->fcn_ == protocol && xc->cid_ == cid; } );

            if ( it == results_.end() ) {
                results_.emplace_back( std::make_shared< xChromatogram >( m, width, protocol, cid, display_name, ms.isHistogram() ) );
                it = results_.end() - 1;
            }
            ( *it )->append( uint32_t( pos ), time, y.get() );
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

        if ( auto y = computeIntensity( ms, adcontrols::hor_axis_mass, std::make_pair( lMass, uMass ) ) ) {

            auto it = std::find_if( results_.begin(), results_.end(), [=]( std::shared_ptr<xChromatogram>& xc ) { return xc->fcn_ == protocol && xc->cid_ == cid; } );

            if ( it == results_.end() ) {
                results_.emplace_back( std::make_shared< xChromatogram >( protocol, cid, ms.isHistogram() ) );
                it = results_.end() - 1;
                ( *it )->pChr_->addDescription(
                    adcontrols::description(
                        L"Create"
                        , ( boost::wformat( L"%s m/z %.4lf(W:%.4gmDa)_%d" )
                            % utf::to_wstring( display_name ) % pk.mass() % pk.widthHH() % protocol ).str() ) );
            }
            ( *it )->append( uint32_t( pos ), time, y.get() );
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
    if ( auto y = computeIntensity( ms, axis, range ) ) {

        auto it = std::find_if( results_.begin(), results_.end(), [=]( std::shared_ptr<xChromatogram>& xc ) { return xc->fcn_ == protocol && xc->cid_ == cid; } );

        if ( it == results_.end() ) {
            results_.emplace_back( std::make_shared< xChromatogram >( protocol, cid, ms.isHistogram() ) );
            it = results_.end() - 1;
            double value_width = range.second - range.first;
            double value = range.first + value_width;
            if ( axis == adcontrols::hor_axis_mass ) {
                ( *it )->pChr_->addDescription( adcontrols::description(
                                                    L"Create"
                                                    , ( boost::wformat( L"%s m/z %.4lf(W:%.4gmDa)_%d" )
                                                        % utf::to_wstring( display_name ) % value % (value_width * 1000) % protocol ).str() ) );
            } else {
                ( *it )->pChr_->addDescription( adcontrols::description(
                                                    L"Create"
                                                    , ( boost::wformat( L"%s %.4lfus(W:%.4gns)_%d" )
                                                        % utf::to_wstring( display_name ) % (value*std::micro::den) % (value_width*std::nano::den) % protocol ).str() ) );
            }
        }
        ( *it )->append( uint32_t( pos ), time, y.get() );
    }
}

bool
MSChromatogramExtractor::impl::apply_mslock( std::shared_ptr< adcontrols::MassSpectrum > profile
                                             , const adcontrols::ProcessMethod& pm
                                             , adcontrols::lockmass::mslock& mslock )
{
    // lock using previous (auto targeting) result -- workaround for non dual spray acquisition
    if ( mslock_ ) {
        mslock = mslock_;
        mslock_( *profile );
        return true;
    }

    if ( auto cm = pm.find< adcontrols::CentroidMethod >() ) {
        adcontrols::MassSpectrum centroid;
        doCentroid( centroid, *profile, *cm );

        if ( msLocker_ ) {
            // find internal MS reference
            if ( auto lock = (*msLocker_)( centroid ) ) {
                mslock = *lock;
                if ( *lock ) {
                    (*lock)( *profile );
                    // ADDEBUG() << "lock with internal reference";
                    return true;
                } else {
                    // ADDEBUG() << "internal reference not found";
                }
            }
        }
    }

    return false;
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

const std::vector< std::pair< int64_t, std::array<double, 2> > >&
MSChromatogramExtractor::lkms() const
{
    return impl_->lkms_;
}

// bool
// MSChromatogramExtractor::impl::doMSLock( adcontrols::lockmass::mslock& mslock
//                                          , const adcontrols::MassSpectrum& centroid )
// {
//     if ( msLocker_ ) {
//         if ( auto lock = (*msLocker_)( centroid ) ) {
//             mslock = *lock;
//             return true;
//         }
//     }
// #if 0
//     adcontrols::MSFinder find( m.tolerance( m.toleranceMethod() ), m.algorithm(), m.toleranceMethod() );

//     int mode = (-1);  // TODO: lock mass does not support rapid protocol

//     for ( auto& msref : msrefs_ ) {
//         size_t proto = 0;
//         for ( auto& fms: adcontrols::segment_wrapper< const adcontrols::MassSpectrum >( centroid ) ) {
//             size_t idx = find( fms, msref.second );
//             if ( idx != adcontrols::MSFinder::npos ) {
//                 if ( mode < 0 )
//                     mode = fms.mode();
//                 if ( mode == fms.mode() ) {
//                     mslock << adcontrols::lockmass::reference( msref.first, msref.second, fms.getMass( idx ), fms.getTime( idx ) );
//                     // ADDEBUG() << "found ref: " << msref << "@ mode=" << mode << " proto=" << proto;
//                 } else {
//                     ADDEBUG() << "found ref: " << msref << " but mode does not match.";
//                 }
//             } else {
//                 // ADDEBUG() << "msref " << msref << " not found.";
//             }
//             ++proto;
//         }
//     }

//     if ( mslock.fit() )
//         return true;
// #endif
//     return false;
// }
