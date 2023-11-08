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
#include <adcontrols/genchromatogram.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/moltable.hpp>
#include <adcontrols/mschromatogrammethod.hpp>
#include <adcontrols/msfinder.hpp>
#include <adcontrols/mslockmethod.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/quan/extract_by_mols.hpp>
#include <adcontrols/quanresponsemethod.hpp>
#include <adcontrols/segment_wrapper.hpp>
#include <adcontrols/targeting.hpp>
#include <adcontrols/waveform_filter.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/date_time.hpp>
#include <adportable/debug.hpp>
#include <adportable/json/extract.hpp>
#include <adportable/json_helper.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adportable/unique_ptr.hpp>
#include <adportable/utf.hpp>
#include <adutils/acquiredconf.hpp>
#include <boost/format.hpp>
#include <boost/json.hpp>
#include <boost/optional.hpp>
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
        impl( const adcontrols::LCMSDataset * raw
              , std::shared_ptr< dataprocessor > dp) : raw_( raw )
                                                     , processor_( dp )
            {}

        bool apply_mslock( std::shared_ptr< adcontrols::MassSpectrum >, const adcontrols::ProcessMethod&, adcontrols::lockmass::mslock& );
        void create_chromatograms( std::vector< std::shared_ptr< adcontrols::Chromatogram > >& vec
                                   , const adcontrols::MSChromatogramMethod& m );

        // [0]
        void append_to_chromatogram( size_t pos, const adcontrols::MassSpectrum& ms, const adcontrols::MSChromatogramMethod&, const std::string& );

        // [1]
        void append_to_chromatogram( size_t pos, const adcontrols::MassSpectrum& ms, const adcontrols::MSPeakInfo&, const std::string&, double width );

        // [2]
        void append_to_chromatogram( size_t pos, const adcontrols::MassSpectrum& ms, adcontrols::hor_axis, const std::pair<double, double>& range, const std::string& );

        bool doCentroid( adcontrols::MassSpectrum& centroid, const adcontrols::MassSpectrum& profile, const adcontrols::CentroidMethod& );

        std::optional< adcontrols::description > desc_mslock() const {
            if ( lkms_.size() ) {
                auto jv = boost::json::value{{ "mslock", {{ "method", "internal" }} }}; // on-tye-fly
                return adcontrols::description( { "MSLock", boost::json::serialize( jv ) } );
            } else if ( mslock_ ) {
                auto jv = boost::json::value{{ "mslock", {{ "method", "internal" }, { "data", boost::json::value_from(mslock_)}} }};
                return adcontrols::description( { "MSLock", boost::json::serialize( jv ) } );
            } else if ( auto global_mslock = processor_->dataGlobalMSLock() ) {
                auto jv = boost::json::value{{ "mslock", {{"method", "external"}, {"data", boost::json::value_from(*global_mslock)}} }};
                return adcontrols::description( { "MSLock", boost::json::serialize( jv ) } );
            }
            return {};
        }

        std::vector< std::shared_ptr< mschromatogramextractor::xChromatogram > > results_; // vector<chromatogram>
        std::map< int, std::vector< std::shared_ptr< mschromatogramextractor::xChromatogram > > > xresults_; // fcn, vector<chromatogram>

        std::map< size_t, std::shared_ptr< adcontrols::MassSpectrum > > spectra_;
        const adcontrols::LCMSDataset * raw_;
        std::shared_ptr< adcontrols::CentroidMethod > centroidMetod_;
        //
        std::unique_ptr< msLocker > msLocker_;
        adcontrols::lockmass::mslock mslock_; // mslock at auto-targeting
        std::vector< std::pair< int64_t, std::array< double, 2 > > > lkms_;  // time, coeffs
        //
        std::shared_ptr< dataprocessor > processor_;
    };

    struct protocol_finder {
        boost::optional< int > operator()( std::shared_ptr< const adcontrols::MassSpectrum > ms
                                           , const adcontrols::moltable::value_type& mol, double width ) {
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
        double yoffs;
        std::shared_ptr< adcontrols::Chromatogram > pChr;

        cXtractor( double _m
                   , double _w
                   , double _l
                   , double _u
                   , int32_t _p
                   , const std::string& desc = "" ) : mass( _m )
                                                    , width( _w )
                                                    , lMass( _l )
                                                    , uMass( _u )
                                                    , proto( _p )
                                                    , yoffs( 0 )
                                                    , pChr( std::make_shared< adcontrols::Chromatogram >() ) {
            if ( ! desc.empty() )
                pChr->addDescription( adcontrols::description( { "create", desc } ) );
            pChr->setProtocol( _p );
        }

        inline void append( uint32_t pos, double time, double y ) {
            if ( pChr->size() == 0 ) {
                yoffs = y;
            }
            (*pChr) << std::make_pair( time, y - yoffs );
        }
        inline void append( uint32_t pos, double time, double y, double tof, double mass ) {
            if ( pChr->size() == 0 ) {
                yoffs = y;
            }
            (*pChr) << std::make_tuple( time, y - yoffs, tof, mass );
        }
    };
}

namespace {
    std::string remove_html_tag( std::string text ) {
        std::string::size_type pos;
        while ((pos = text.find( "<" )) != std::string::npos ) {
            auto endpos = text.find( ">", pos );
            if ( endpos != std::string::npos )
                text.erase( pos, endpos - pos + 1 );
        }
        return text;
    }
}

using namespace adprocessor::v3;

MSChromatogramExtractor::~MSChromatogramExtractor()
{
    delete impl_;
}
MSChromatogramExtractor::MSChromatogramExtractor( const adcontrols::LCMSDataset * raw
                                                  , dataprocessor * dp ) : impl_( new impl( raw, dp->shared_from_this() ) )
{
}

std::shared_ptr< const adcontrols::MassSpectrum >
MSChromatogramExtractor::getMassSpectrum( double tR ) const
{
    auto it = std::lower_bound( impl_->spectra_.begin()
                                , impl_->spectra_.end()
                                , tR
                                , [&]( const auto& pair, double t ){ return pair.second->getMSProperty().timeSinceInjection() < t; });
    if ( it == impl_->spectra_.end() )
        return nullptr;
    return it->second;
}

bool
MSChromatogramExtractor::loadSpectra( const adcontrols::ProcessMethod * pm
                                      , std::shared_ptr< const adcontrols::DataReader > reader
                                      , int fcn
                                      , std::function<bool( size_t, size_t )> progress
                                      , size_t nCount
                                      , size_t& nProg )
{
    const size_t nSpectra = reader->size( fcn );

    if ( nSpectra == 0 )
        return false;

    progress( nProg, nCount );

    impl_->results_.clear();
    impl_->msLocker_.reset();
    const adcontrols::MSChromatogramMethod * cm = pm ? pm->find< adcontrols::MSChromatogramMethod >() : nullptr;
    if ( cm->lockmass() )
        impl_->msLocker_ = std::make_unique< msLocker > ( *cm, *pm );

    adcontrols::lockmass::mslock mslock;
    auto global_mslock = impl_->processor_->dataGlobalMSLock();

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
        } else if ( global_mslock ) {
            (*global_mslock)( *ms );
        }

        impl_->spectra_[ it->pos() ] = ms; // (:= pos sort order) keep mass locked spectral series
        if ( progress( ++nProg, nCount ) )
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
                msfinder = std::make_unique< adcontrols::MSFinder >( qrm->width()
                                                                     , qrm->findAlgorithm(), adcontrols::idToleranceDaltons );
            }
        }
    }

    // ADDEBUG() << "##################### extract_by_mols ##########################";
    auto global_mslock = impl_->processor_->dataGlobalMSLock();
    ADDEBUG() << "######## extract_by_mols ######### " << (global_mslock ? "Has global_mslock" : "no lock");

    if ( auto cm = pm.find< adcontrols::MSChromatogramMethod >() ) {
        const auto polarity = cm->molecules().polarity();
        std::vector< cXtractor > temp;
        auto it = reader->begin( -1 );

        if ( auto sp = reader->readSpectrum( it ) ) {

            if ( global_mslock )
                (*global_mslock)( *sp );

            for ( auto& mol: cm->molecules().data() ) {

                auto proto = mol.protocol();
                if ( proto && mol.enable() ) {
                    double width = cm->width_at_mass( mol.mass() );
                    double lMass = mol.mass() - width / 2;
                    double uMass = mol.mass() + width / 2;
                    auto ion_form = adcontrols::ChemicalFormula::formatAdduct( mol.adducts( polarity ) );
                    auto display_name = ( boost::format( "%s %s (%.3f)" )
                                          % ( mol.synonym().empty() ? mol.formula() : mol.synonym() )
                                          % ion_form
                                          % mol.mass() ).str();
                    auto desc = ( boost::format( "%s %s %.4f (W:%.4gmDa) %s %d" )
                                  % ( mol.synonym().empty() ? mol.formula() : mol.synonym() )
                                  % ion_form
                                  % mol.mass()
                                  % ( width * 1000 )
                                  % reader->display_name()
                                  % proto.get() ).str();

                    adcontrols::quan::extract_by_mols extract_by_mols;

                    if ( ! targets.empty() ) {
                        auto it = std::find_if( targets.begin(), targets.end()
                                                , [&]( const auto& t ){ return t.mol() == mol; });
                        if ( it != targets.end() ) {
                            if ( auto candidate = (*it)[ 0 ] ) {
                                lMass = candidate->mass - width / 2;
                                uMass = candidate->mass + width / 2;
                                desc = ( boost::format( "%s %.4f AT (W:%.4gmDa) %s %d" )
                                         % ( mol.synonym().empty() ? mol.formula() : mol.synonym() )
                                         % candidate->mass
                                         % ( width * 1000 )
                                         % reader->display_name()
                                         % proto.get() ).str();
                                extract_by_mols.auto_target_candidate =
                                    adcontrols::quan::targeting_candidate( candidate->mass
                                                                           , candidate->mass - it->mol().mass()
                                                                           , candidate->idx
                                                                           , candidate->fcn
                                                                           , candidate->charge
                                                                           , candidate->formula );
                            }
                        } else {
                            ADDEBUG() << "=================== target NOT FOUND ===================";
                        }
                    }

                    auto& t = adcontrols::segment_wrapper< const adcontrols::MassSpectrum >( *sp )[ proto.get() ];
                    double tof = t.time( t.getIndexFromMass( mol.mass() ) );
                    auto molid = mol.molid(); // property< boost::uuids::uuid >( "molid" );
                    auto time_of_injection = this->time_of_injection();

                    extract_by_mols.moltable_  = { *proto, mol.mass(), width, mol.formula(), tof, mol.adducts( polarity ) };
                    extract_by_mols.molid      = molid ? *molid : boost::uuids::uuid{};
                    extract_by_mols.wform_type = (sp->isCentroid() ? "centroid" : "profile");
                    extract_by_mols.msref      = ( cm->lockmass() ? boost::optional<bool>( mol.isMSRef() ) : boost::none );
                    extract_by_mols.centroid   = ( peak_detector ? boost::optional<std::string>( areaIntensity ? "area" : "height" ) : boost::none );

                    boost::json::object top = {
                        { "mass", mol.mass() }
                        , { "mass_width", width }
                        , { "protocol",  proto ? *proto : 0 }
                        , { "reader", reader->display_name() }
                            , { "generator"
                            , {{ "time_of_injection", adportable::date_time::to_iso< std::chrono::nanoseconds >( time_of_injection ) }
                               , { "extract_by_mols", boost::json::value_from( extract_by_mols ) }
                            }
                        }
                    };

                    temp.emplace_back( mol.mass(), width, lMass, uMass, (proto ? proto.get() : -1), desc );
                    temp.back().pChr->setGeneratorProperty( boost::json::serialize( top ) );
                    temp.back().pChr->set_display_name( display_name );
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

        size_t nCount = reader->size( -1 ) * 2;
        size_t nProg(0);
        // Generate chromatograms
        if ( loadSpectra( &pm, reader, -1, progress, nCount, nProg ) ) {
            // histogram.timecount.1.u5303a.ms-cheminfo.com
            // tdcdoc.waveform.1.u5303a.ms-cheminfo.com
            const bool isCounting = std::regex_search( reader->objtext(), std::regex( "^histogram.*$|^pkd\\.[1-9]\\.u5303a\\.ms-cheminfo.com" ) );

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
                progress( ++nProg, nCount );
            }

            std::pair< double, double > time_range =
                std::make_pair( impl_->spectra_.begin()->second->getMSProperty().timeSinceInjection()
                                , impl_->spectra_.rbegin()->second->getMSProperty().timeSinceInjection() );

            for ( auto& xc : temp ) {
                xc.pChr->setIsCounting( isCounting );
                xc.pChr->minimumTime( time_range.first );
                xc.pChr->maximumTime( time_range.second );
                ADDEBUG() << "\tcreating chromatogram: " << (global_mslock ? "has lock" : "no lock");
                if ( auto desc = impl_->desc_mslock() ) {
                    ADDEBUG() << "\tdescreptor: " << desc->keyValue();
                    xc.pChr->addDescription( *desc );
                }
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

    double width = 0;
    if ( const adcontrols::MSChromatogramMethod * cm = pm.find< adcontrols::MSChromatogramMethod >() )
        width = cm->width( cm->widthMethod() );

    const size_t nCounts = reader->size( -1 ) * 2;
    size_t nProg(0);

    if ( loadSpectra( &pm, reader, -1, progress, nCounts, nProg ) ) {

        const bool isCounting = std::regex_search( reader->objtext(), std::regex( "^histogram.*$|^pkd\\.[1-9]\\.u5303a\\.ms-cheminfo.com" ) );

        for ( auto& ms : impl_->spectra_ ) {
            for ( const auto& info: adcontrols::segment_wrapper< const adcontrols::MSPeakInfo >( *pkinfo ) ) {
                if ( info.protocolId() == ms.second->protocolId() ) {
                    impl_->append_to_chromatogram( ms.first, *ms.second, info, reader->abbreviated_display_name(), width );
                }
            }
            progress( ++nProg, nCounts );
        }

        std::pair< double, double > time_range =
            std::make_pair( impl_->spectra_.begin()->second->getMSProperty().timeSinceInjection()
                            , impl_->spectra_.rbegin()->second->getMSProperty().timeSinceInjection() );

        for ( auto& r : impl_->results_ ) {
            r->pChr_->setIsCounting( isCounting );
            r->pChr_->minimumTime( time_range.first );
            r->pChr_->maximumTime( time_range.second );
            r->pChr_->setAxisLabel( adcontrols::plot::yAxis, r->isCounting_ ? "Counts" : "Intensity" );
            r->pChr_->setAxisUnit( r->isCounting_ ? adcontrols::plot::Counts : adcontrols::plot::Arbitrary );
            if ( auto desc = impl_->desc_mslock() ) {
                r->pChr_->addDescription( *desc );
            }
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

    size_t nCounts = reader->size( fcn ) * 2;
    size_t nProg(0);
    if ( loadSpectra( &pm, reader, fcn, progress, nCounts, nProg ) ) {
        const bool isCounting = std::regex_search( reader->objtext(), std::regex( "^histogram.*$|^pkd\\.[1-9]\\.u5303a\\.ms-cheminfo.com" ) );

        for ( auto& ms : impl_->spectra_ ) {
            // [2]
            impl_->append_to_chromatogram( ms.first /*pos */, *ms.second, axis, range, reader->abbreviated_display_name() );
            progress( ++nProg, nCounts );
        }

        std::pair< double, double > time_range =
            std::make_pair( impl_->spectra_.begin()->second->getMSProperty().timeSinceInjection()
                          , impl_->spectra_.rbegin()->second->getMSProperty().timeSinceInjection() );
        auto gen = boost::json::value{
            { "mass", range.first + (range.second - range.first ) / 2.0 }
            , { "mass_width", range.second - range.first }
            , { "protocol", fcn }
            , { "reader", reader->abbreviated_display_name() }
            , { "generator"
                , {{ "extract_by_axis_range", {{ "axis", unsigned(axis) }, { "range", boost::json::value_from( range ) }} }
                   , { "reader", { "name", reader->objtext() }, { "protocol", fcn } }
                }
            }
        };

        for ( auto& r : impl_->results_ ) {
            r->pChr_->setIsCounting( isCounting );
            r->pChr_->minimumTime( time_range.first );
            r->pChr_->maximumTime( time_range.second );
            r->pChr_->setAxisLabel( adcontrols::plot::yAxis, r->isCounting_ ? "Counts" : "Intensity" );
            r->pChr_->setAxisUnit( r->isCounting_ ? adcontrols::plot::Counts : adcontrols::plot::Arbitrary );
            r->pChr_->setGeneratorProperty( boost::json::serialize( gen ) );
            if ( auto desc = impl_->desc_mslock() ) {
                r->pChr_->addDescription( *desc );
            }
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

    const std::string wkey = (axis == adcontrols::hor_axis_mass) ? "mass" : "time";

    std::vector< std::tuple< std::pair< double, double >                      // 0 range
                             , int                                            // 1 fcn (proto)
                             , boost::optional< adcontrols::GenChromatogram > // 2
                             > > list;  // pair< range >, fcn
    std::vector< std::string > mols;

    if ( auto formulae = obj.if_contains( "formulae" ) ) {
        std::vector< adcontrols::GenChromatogram > genChromatograms;
        try {
            genChromatograms = boost::json::value_to< std::vector< adcontrols::GenChromatogram > >( *formulae );

            for ( const auto& gen: genChromatograms ) {
                if ( gen.selected ) {
                    double centre = (axis == adcontrols::hor_axis_mass) ? gen.mass : gen.time;
                    list.emplace_back( std::make_pair( centre - width / 2, centre + width / 2 ), gen.proto, gen );
                    mols.emplace_back( gen.formula );
                }
            }
        } catch ( std::exception& ex ) {
            ADDEBUG() << "Error: " << ex.what();
            return false;
        }
    }

    size_t nCounts = reader->size( -1 );
    size_t nProg(0);

    if ( loadSpectra( &pm, reader, -1, progress, nCounts, nProg ) ) {

        const bool isCounting = std::regex_search( reader->objtext(), std::regex( "^histogram.*$|^pkd\\.[1-9]\\.u5303a\\.ms-cheminfo.com" ) );
        // ADDEBUG() << "########## isCounting: " << isCounting << ", list.size = " << list.size();

        auto fmt = ( axis == adcontrols::hor_axis_mass ) ? boost::format( "%s %.1f(W:%.1fmDa) %s,p%d" ) : boost::format( "%s %.4lfus(W:%.1ns) %s,p%d" );

        for ( size_t idx = 0; idx < list.size(); ++idx ) {
            auto res = std::make_shared< mschromatogramextractor::xChromatogram >( /*fcn*/ std::get< 1 >( list[ idx ] ), idx, isCounting );
            int protocol = std::get< 1 >( list[ idx ] );
            double width = std::get< 0 >( list[ idx ] ).second - std::get< 0 >( list[ idx ] ).first;
            double centre = std::get< 0 >( list[ idx ] ).first + width / 2.0;
            const std::string& formula = mols[ idx ];
            if ( axis == adcontrols::hor_axis_mass ) {
                width *= 1000;             // --> mDa
            } else {
                centre *= std::micro::den; // --> us
                width *= std::nano::den;   // --> ns
            }

            auto gen = std::get< 2 >( list[ idx ] );
            res->pChr_->addDescription(
                adcontrols::description( { "create", ( fmt
                                                       % (gen ? remove_html_tag( gen->display_name ) : formula )
                                                       % centre
                                                       % width
                                                       % reader->abbreviated_display_name()
                                                       % protocol ).str() } ) );
            if ( auto desc = impl_->desc_mslock() ) {
                res->pChr_->addDescription( *desc );
            }

            res->pChr_->setIsCounting( res->isCounting_ );
            if ( gen ) {
                res->pChr_->setGeneratorProperty( boost::json::serialize( boost::json::value_from( *gen ) ) );
                res->pChr_->set_display_name( gen->display_name );
            }
            impl_->results_.emplace_back( res );
        }

        // compute each point on the chromatogram
        for ( auto& ms : impl_->spectra_ ) {
            size_t cid(0);
            for ( const auto& item: list ) {
                const int proto = std::get< 1 >( item );
                const uint32_t pos = ms.first;
                if ( const auto pms = ms.second->findProtocol( proto ) ) {
                    double time = pms->getMSProperty().timeSinceInjection();
                    auto y = computeIntensity( *pms, axis, std::get< 0 >( item ) );
                    impl_->results_[ cid ]->append( pos, time, y ? y.get() : 0 );
                } else {
                    double time = ms.second->getMSProperty().timeSinceInjection();
                    impl_->results_[ cid ]->append( pos, time, 0 ); // assin zero if no data found.
                }
                ++cid;
            }
            progress( ++nProg, nCounts );
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

            if ( ms.mass( 0 ) <= lMass && uMass < ms.mass( ms.size() - 1 ) ) {
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
                    double t0 = ms.time( fraction.lPos );
                    double t1 = ms.time( fraction.lPos + 1 );
                    assert( t0 < lTime && lTime < t1 );
                    fraction.lFrac = ( t1 - lTime ) / ( t1 - t0 );
                }
                {
                    double t0 = ms.time( fraction.uPos );
                    double t1 = ms.time( fraction.uPos + 1 );
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

            auto it = std::find_if( results_.begin(), results_.end()
                                    , [=]( std::shared_ptr<xChromatogram>& xc ) { return xc->fcn_ == protocol && xc->cid_ == cid; } );

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
                                                       , const std::string& display_name
                                                       , double width )
{
    using namespace mschromatogramextractor;
    using adportable::utf;

    if ( ms.protocolId() != pkinfo.protocolId() )
        return;

    const int protocol = ms.protocolId();
    const double time = ms.getMSProperty().timeSinceInjection();

    uint32_t cid = 0;

    for ( auto& pk : pkinfo ) {

        double lMass = (width < 0.001) ? pk.mass() - pk.widthHH() / 2 : pk.mass() - width / 2.0;
        double uMass = (width < 0.001) ? pk.mass() + pk.widthHH() / 2 : pk.mass() + width / 2.0;

        if ( auto y = computeIntensity( ms, adcontrols::hor_axis_mass, std::make_pair( lMass, uMass ) ) ) {

            auto it = std::find_if( results_.begin(), results_.end()
                                    , [=]( std::shared_ptr<xChromatogram>& xc ) { return xc->fcn_ == protocol && xc->cid_ == cid; } );

            if ( it == results_.end() ) {
                results_.emplace_back( std::make_shared< xChromatogram >( protocol, cid, ms.isHistogram() ) );
                it = results_.end() - 1;
                ( *it )->pChr_->addDescription(
                    adcontrols::description(
                        {"create"
                         , ( boost::format( "m/z %.3lf(W %.1fmDa),%s,p%d" )
                             % pk.mass() % ((uMass - lMass) * 1000) % display_name % protocol ).str()} ) );
                    //--------- add property ---------
                boost::system::error_code ec;
                auto jv = boost::json::parse( pk.toJson(), ec );
                if ( !ec ) {
                    boost::json::object obj = {
                        { "mass", pk.mass() }
                        , { "mass_width", width }
                        , { "protocol", protocol }
                        , { "reader", display_name }
                        , { "generator"
                            , {{ "extract_by_peak_info", {{ "pkinfo", jv }} }}
                        }
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
                                                    { "create"
                                                      , ( boost::format( "m/z %.4lf(W:%.4gmDa) %s,p%d" )
                                                          % value
                                                          % (value_width * 1000)
                                                          % display_name
                                                          % protocol ).str() }) );
            } else {
                // time axis
                ( *it )->pChr_->addDescription( adcontrols::description(
                                                    { "create"
                                                      , ( boost::format( "%.4lfus(W:%.4gns) %s,p%d" )
                                                          % (value*std::micro::den)
                                                          % (value_width*std::nano::den)
                                                          % display_name
                                                          % protocol ).str()} ) );
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
        mslock_.setProperty( { "MSLock", "Targeting" } );
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

const std::map< size_t, std::shared_ptr< adcontrols::MassSpectrum > >
MSChromatogramExtractor::spectra() const
{
    return impl_->spectra_;
}

std::optional< adcontrols::description >
MSChromatogramExtractor::mslock_description() const
{
    return impl_->desc_mslock();
}
