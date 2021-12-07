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
#include "autotargetingcandidates.hpp"
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
#include <adcontrols/quan/extract_by_mols.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/date_time.hpp>
#include <adportable/debug.hpp>
#include <adportable/json_helper.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adportable/unique_ptr.hpp>
#include <adportable/utf.hpp>
#include <adutils/acquiredconf.hpp>
#include <boost/format.hpp>
#include <boost/json.hpp>
#include <boost/optional/optional_io.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_serialize.hpp>
#include <algorithm>
#include <numeric>
#include <ratio>
#include <regex>
#include <set>

#include "autotargeting.hpp"
#include "mslocker.hpp"

namespace adprocessor {

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
                    if (  range.first < lMass && uMass < range.second ) {
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
    return extract_by_mols( vec, pm, reader, std::vector< AutoTargetingCandidates >{}, progress );
}

bool
MSChromatogramExtractor::extract_by_mols( std::vector< std::shared_ptr< adcontrols::Chromatogram > >& vec
                                          , const adcontrols::ProcessMethod& pm
                                          , std::shared_ptr< const adcontrols::DataReader > reader
                                          , const std::vector< AutoTargetingCandidates >& targets
                                          , std::function<bool( size_t, size_t )> progress )
{
    vec.clear();
    impl_->spectra_.clear();
    impl_->mslock_ = {};

    if ( impl_->raw_->dataformat_version() <= 2 )
        return false;

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

                auto proto = mol.protocol();
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

                    boost::json::object auto_target_candidate;
                    if ( ! targets.empty() ) {
                        auto it = std::find_if( targets.begin(), targets.end(), [&]( const auto& t ){ return t.mol() == mol; });
                        if ( it != targets.end() ) {
                            if ( auto candidate = (*it)[ 0 ] ) {
                                lMass = candidate->mass - width / 2;
                                uMass = candidate->mass + width / 2;
                                desc = ( boost::wformat( L"%s %.4f AT (W:%.4gmDa) %s %d" )
                                         % adportable::utf::to_wstring( mol.formula() )
                                         % candidate->mass
                                         % ( width * 1000 )
                                         % adportable::utf::to_wstring( reader->display_name() )
                                         % proto.get() ).str();
                                adcontrols::quan::targeting_candidate atc( candidate->mass
                                                                           , candidate->mass - it->mol().mass()
                                                                           , candidate->idx
                                                                           , candidate->fcn
                                                                           , candidate->charge
                                                                           , candidate->formula );
                                auto_target_candidate = boost::json::value_from( std::move( atc ) ).as_object();
                            }
                        } else {
                            ADDEBUG() << "=================== target NOT FOUND ===================";
                        }
                    }

                    auto& t = adcontrols::segment_wrapper< const adcontrols::MassSpectrum >( *sp )[ proto.get() ];
                    double tof = t.getTime( t.getIndexFromMass( mol.mass() ) );
                    auto time_of_injection = this->time_of_injection();

                    auto molid = mol.property< boost::uuids::uuid >( "molid" ); // optional
                    adcontrols::quan::moltable moltable( *proto, mol.mass(), width, mol.formula(), tof );

                    boost::json::object top = {
                        { "generator"
                          , {   { "time_of_injection", adportable::date_time::to_iso< std::chrono::nanoseconds >( time_of_injection ) }
                              , { "extract_by_mols",
                                  {  { "molid", boost::uuids::to_string( molid ? *molid : boost::uuids::uuid{} ) } // only if quan
                                   , { "wform_type", (sp->isCentroid() ? "centroid" : "profile") }
                                   , { "moltable", boost::json::value_from( moltable ) }
                                   , { "auto_target_candidate", auto_target_candidate }
                                   , { "msref", ( cm->lockmass() ? mol.isMSRef() : boost::json::value{} ) }
                                   , { "centroid", ( peak_detector ? (areaIntensity ? "area" : "height") : boost::json::value{} ) }
                                  }
                                }
                            }}};

                    ADDEBUG() << "---------------------------------------------------------\n" << top;
                    {
                        auto temp = boost::json::value_to< adcontrols::quan::extract_by_mols >( adportable::json_helper::find( top, "generator.extract_by_mols" ) );
                        ADDEBUG() << "\nmolid: " << temp.molid;
                        ADDEBUG() << "wform_type: " << temp.wform_type;
                        ADDEBUG() << "moltable: " << boost::json::value_from( temp.moltable_ );
                        ADDEBUG() << "auto_target_candidate: " << (temp.auto_target_candidate ? "exist" : "not exist");
                        ADDEBUG() << "msref: " << temp.msref;
                        ADDEBUG() << "centroid: " << temp.centroid;
                        ADDEBUG() << "---------------------------------------------------------\n";

                        auto jv = boost::json::value_from( temp );
                        ADDEBUG() << "------------ verify ---------------------------------------------\n" << jv;
                    }

                    temp.emplace_back( mol.mass(), width, lMass, uMass, (proto ? proto.get() : -1), desc );
                    temp.back().pChr->setGeneratorProperty( boost::json::serialize( top ) );
                    //
                    temp.back().pChr->set_time_of_injection( std::move( time_of_injection ) );
                    if ( sp->isHistogram() ) {
                        temp.back().pChr->setAxisLabel( adcontrols::plot::yAxis, "Counts" );
                        temp.back().pChr->setAxisUnit( adcontrols::plot::Counts );
                    } else {
                        temp.back().pChr->setAxisLabel( adcontrols::plot::yAxis, areaIntensity ? "Intensity (area)" : "Intensity" );
                        temp.back().pChr->setAxisUnit( adcontrols::plot::Volts, 1000 ); // mV
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
    auto obj = boost::json::parse( json ).as_object();
    // ADDEBUG() << "## " << __FUNCTION__ << "\n" << obj;

    const std::string wkey = (axis == adcontrols::hor_axis_mass) ? "mass" : "time";

    std::vector< std::pair< std::pair< double, double >, int > > list;  // pair< range >, fcn
    std::vector< std::string > mols;

    if ( auto formulae = obj.if_contains( "formulae" ) ) {
        for ( auto line: formulae->as_array() ) {
            if ( auto selected = line.as_object().if_contains( "selected" ) ) {
                if ( selected->as_bool() ) {
                    int proto = 0;
                    if ( auto item = line.as_object().if_contains( "protocol" ) ) {
                        proto = item->as_int64();
                    } else if ( auto item = line.as_object().if_contains( "proto" ) ) {
                        proto = item->as_int64();
                    }
                    if ( auto item = line.as_object().if_contains( wkey ) ) {
                        double centre = item->as_double();
                        if ( auto formula = line.as_object().if_contains( "formula" ) ) {
                            list.emplace_back( std::make_pair( centre - width / 2, centre + width / 2 ), proto );
                            mols.emplace_back( formula->as_string().data() );
                        }
                    }
                }
            }
            if ( auto children = line.as_object().if_contains("children") ) {
                for ( auto child: children->as_array() ) {
                    if ( auto selected = child.as_object().if_contains( "selected" ) ) {
                        int proto = 0;
                        if ( auto pno = child.as_object().if_contains( "protocol" ) ) {
                            proto = pno->as_int64();
                        } else if ( auto pno = child.as_object().if_contains( "proto" ) ) {
                            proto = pno->as_int64();
                        }
                        if ( auto item = child.as_object().if_contains( wkey ) ) {
                            double centre = item->as_double();
                            if ( auto formula = line.as_object().if_contains( "formula" ) ) {
                                list.emplace_back( std::make_pair( centre - width / 2, centre + width / 2 ), proto );
                                mols.emplace_back( formula->as_string().data() );
                            }
                        }
                    }
                }
            }
        }
    }

    if ( loadSpectra( &pm, reader, -1, progress ) ) {

        const bool isCounting = std::regex_search( reader->objtext(), std::regex( "^pkd\\.[1-9]\\.u5303a\\.ms-cheminfo.com" ) ); // pkd is counting
        ADDEBUG() << "########## isCounting: " << isCounting << ", list.size = " << list.size();

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
            // // debug
            // boost::json::object obj{
            //     { { "idx", idx }, { "formula", formula }, { "width", width }, { "centre", centre }, { "isCounting", isCounting } }
            // };
            // ADDEBUG() << boost::json::serialize( obj );
            // // end debug

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
                        , ( boost::wformat( L"m/z %.3lf(W %.1fmDa),%s_%d" )
                            % pk.mass() % (pk.widthHH() * 1000) % utf::to_wstring( display_name ) % protocol ).str() ) );
                //--------- add property ---------
                boost::system::error_code ec;
                auto jv = boost::json::parse( pk.toJson(), ec );
                if ( !ec ) {
                    boost::json::object obj = {
                        { "generator"
                          , {{ "extract_by_peak_info"
                                  , {{ "pkinfo", jv }}
                                }} }
                    };
                    ( *it )->pChr_->setGeneratorProperty( boost::json::serialize( obj ) );
                }

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
                    return true;
                } else {
                    ADDEBUG() << "## " << __FUNCTION__ << " ##\tinternal reference not found";
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
    }

    if ( profile.numSegments() > 0 ) {
        for ( size_t fcn = 0; fcn < profile.numSegments(); ++fcn ) {
            auto temp = std::make_shared< adcontrols::MassSpectrum >();
            result |= peak_detector( profile.getSegment( fcn ) );
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
