// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2025 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2025 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "mzmlspectrum.hpp"
#include "accession.hpp"
#include "binarydataarray.hpp"
//#include "mzmlchromatogram.hpp"
#include "mzmldatumbase.hpp"
#include "mzmlreader.hpp"
#include "serializer.hpp"
#include "scan_protocol.hpp"
#include "xmltojson.hpp"
#include "mzmlwalker.hpp"
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adportable/debug.hpp>
#include <boost/json.hpp>
#include <pugixml.hpp>
#include <sstream>
#include <variant>

#if 0
namespace {
    struct reader {
        static mzml::datum_variant_t read( const pugi::xml_node& node ) {
            return mzml::mzMLReader{}( node );
            mzml::mzMLDatumBase b{};
            mzml::mzMLWalker z{};
            mzml::mzMLSpectrum y{};
            mzml::mzMLChromatogram x{};
            return {};
        }
    };
}
#endif

namespace mzml {

    class mzMLSpectrum::impl {
        mzMLSpectrum * this_;
    public:
        impl( mzMLSpectrum * pThis ) : this_( pThis ) {}
        impl( mzMLSpectrum * pThis
              , binaryDataArray prime
              , binaryDataArray secondi ) : this_( pThis )
                                          , prime_( prime )
                                          , secondi_( secondi )
                                          , scan_id_( scan_identifier{}( this_->node() ) )
                                          , protocol_number_( 0 )
                                          , highest_observed_mz_ ( 0 )
                                          , lowest_observed_mz_( 0 ) {
        }
        impl( mzMLSpectrum * pThis
              , const impl& t ) : this_( pThis )
                                , prime_( t.prime_ )
                                , secondi_( t.secondi_ )
                                , scan_id_( t.scan_id_ )
                                , protocol_number_( t.protocol_number_ )
                                , highest_observed_mz_( t.highest_observed_mz_ )
                                , lowest_observed_mz_( t.lowest_observed_mz_ ) {
        }

        binaryDataArray prime_;   // mz array
        binaryDataArray secondi_; // intensity array
        mzml::scan_id scan_id_;   // index, id (scan="1"), scan_start_time,
                                  // scan_protocol { ms_level_, precursor_mz_, CE, scan range
        int protocol_number_;
        double highest_observed_mz_;
        double lowest_observed_mz_;
    };

    mzMLSpectrum::~mzMLSpectrum()
    {
    }

    mzMLSpectrum::mzMLSpectrum() : impl_( std::make_unique< impl >( this ) )
    {
        ADDEBUG() << "========== mzMLSpectrum::ctor =============";
    }

    mzMLSpectrum::mzMLSpectrum( const mzMLSpectrum& t )
        : impl_( std::make_unique< impl >( this, *t.impl_ ) )
    {
    }

    mzMLSpectrum::mzMLSpectrum( binaryDataArray prime
                                , binaryDataArray secondi
                                , pugi::xml_node node )
        : mzMLDatumBase( node )
        , impl_( std::make_unique< impl >( this, prime, secondi ) )
    {
        ADDEBUG() << "========== mzMLSpectrum::ctor =============";
    }

    size_t
    mzMLSpectrum::length() const
    {
        return impl_->prime_.length();
    }

    std::pair< const binaryDataArray&, const binaryDataArray& >
    mzMLSpectrum::dataArrays() const
    {
        return { impl_->prime_, impl_->secondi_ };
    }

    boost::json::value
    mzMLSpectrum::to_value() const
    {
        return mzml::to_value{}( node() );
    }

    const scan_id&
    mzMLSpectrum::get_scan_id() const
    {
        return impl_->scan_id_;
    }

    double
    mzMLSpectrum::scan_start_time() const
    {
        return scan_id_accessor{impl_->scan_id_}.scan_start_time();
    }

    std::pair< double, double >
    mzMLSpectrum::scan_range() const // lower, upper
    {
        const auto& proto = scan_id_accessor{ impl_->scan_id_ }.get_scan_protocol();
        return { proto.scan_window_lower_limit(), proto.scan_window_upper_limit() };
    }

    double
    mzMLSpectrum::precursor_mz() const
    {
        return scan_id_accessor{ impl_->scan_id_ }.get_scan_protocol().precursor_mz();
    }

    int
    mzMLSpectrum::ms_level() const
    {
        return scan_id_accessor{ impl_->scan_id_ }.get_scan_protocol().ms_level();
    }

    ion_polarity_type
    mzMLSpectrum::polarity() const
    {
        return scan_id_accessor{ impl_->scan_id_ }.get_scan_protocol().polarity();
    }

    std::pair< double, double >
    mzMLSpectrum::base_peak() const  // mz, intensity
    {
        accession ac( node() );
        if ( auto mass = ac.base_peak_mz() ) {
            if ( auto bpi = ac.base_peak_intensity() ) {
                return { *mass, *bpi };
            }
        }
        return {};
    }

    bool
    mzMLSpectrum::is_profile() const
    {
        return accession( node() ).is_profile_spectrum();
    }

    double
    mzMLSpectrum::total_ion_current() const
    {
        if ( auto tic = accession( node() ).total_ion_current() )
            return *tic;
        return 0;
    }

    void
    mzMLSpectrum::set_protocol_id( int id )
    {
        impl_->protocol_number_ = id;
    }

    int
    mzMLSpectrum::protocol_id() const
    {
        return impl_->protocol_number_;
    }

    std::shared_ptr< adcontrols::MassSpectrum >
    mzMLSpectrum::toMassSpectrum( const mzMLSpectrum& t )
    {
        auto ms = std::make_shared< adcontrols::MassSpectrum >();

        ms->resize( t.length() );
        auto [ masses, intensities ] = t.dataArrays();

        std::visit([&](auto arg){
            for ( size_t i = 0; i < t.length(); ++i )
                ms->setMass( i, *arg++ );
        }, masses.data() );

        std::visit([&](auto arg){
            for ( size_t i = 0; i < t.length(); ++i )
                ms->setIntensity ( i, *arg++ );
        }, intensities.data() );

        const auto& id = t.get_scan_id();
        const auto& proto = scan_id_accessor{ t.get_scan_id() }.get_scan_protocol();

        if ( t.is_profile() )
            ms->setCentroid( adcontrols::CentroidNone );

        ms->setAcquisitionMassRange( proto.scan_window_lower_limit(), proto.scan_window_upper_limit() );
        auto polarity = t.polarity();
        if ( polarity == polarity_negative )
            ms->setPolarity( adcontrols::PolarityNegative );
        else
            ms->setPolarity( adcontrols::PolarityPositive );

        auto& prop = ms->getMSProperty();
        prop.setTimeSinceInjection( t.scan_start_time() );
        prop.setTrigNumber( scan_id_accessor{ t.get_scan_id() }.scan_index() );
        prop.setInstMassRange( { proto.scan_window_lower_limit(), proto.scan_window_upper_limit() } );

        // Shimadzu does not expose timestamp in mzML
        std::chrono::time_point< std::chrono::system_clock, std::chrono::nanoseconds > tp; // epoch
        std::chrono::nanoseconds elapsed_time( int64_t( t.scan_start_time() * 1e9 ) );
        tp += elapsed_time;
        prop.setTimePoint( tp );

        return ms;
    }

    std::string
    mzMLSpectrum::serialize() const
    {
        std::ostringstream o;
        node().print( o );
        return o.str();
    }

#if 0
    namespace {
        template<class... Ts>
        struct overloaded : Ts... { using Ts::operator()...; };
        template<class... Ts>
        overloaded(Ts...) -> overloaded<Ts...>;
    }

    // static
    std::shared_ptr< mzMLSpectrum >
    mzMLSpectrum::deserialize( const char * data, size_t size ) const
    {
        pugi::xml_document doc;
        if ( doc.load_string( data ) ) {
            if ( auto node = doc.select_node( "spectrum" ) ) {
                auto v = reader::read( node.node() );
                return std::visit( overloaded{
                        [](auto&& arg)->std::shared_ptr< mzml::mzMLSpectrum >{ return nullptr; }
                            , [](std::shared_ptr< mzml::mzMLSpectrum >&& sp) { return sp; }
                            }, v);
            }
        }
        return nullptr;
        // return ::mzml::serializer::deserialize( data, size );
        //return ::mzml::serializer::deserialize( data, size );
    }
#endif
} // mzml
