// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2024 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2024 MS-Cheminformatics LLC
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

#include "andims.hpp"
#include "attribute.hpp"
#include "constants.hpp"
#include "datareader.hpp"
#include "ncfile.hpp"
#include "timestamp.hpp"
#include "variable.hpp"
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/baseline.hpp>
#include <adcontrols/baselines.hpp>
#include <adcontrols/peak.hpp>
#include <adcontrols/peaks.hpp>
#include <adportable/debug.hpp>
#include <adportable/iso8601.hpp>
#include <adportable/json_helper.hpp>
#include <boost/json.hpp>
#include <boost/format.hpp>
#include <boost/json/serializer.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <algorithm>
#include <bitset>
#include <memory>
#include <variant>

// ---------------------- overloads -------------------->
namespace {
    // helper type for the visitor #4
    template<class... Ts>
    struct overloaded : Ts... { using Ts::operator()...; };

    // explicit deduction guide (not needed as of C++20)
    template<class... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;
}
//<---------------------- overloads --------------------

namespace adnetcdf {

    enum test_scan_function { Mass_Scan, Selected_Ion_Detection  };
    enum test_ionization_polarity {  Positive_Polarity = adcontrols::polarity_positive
                                     , Negative_Polarity = adcontrols::polarity_negative };
    enum experiment_type { Centroid_Mass_Spectrum, Profile_Mass_Spectrum };

    template< typename T >  struct attribute_values {};
    template<> struct attribute_values< test_scan_function > {
        static constexpr const std::pair< const char * const, test_scan_function > values [] = {
            { "Mass Scan", Mass_Scan }
            , {"Selected Ion Detection", Selected_Ion_Detection} // either SIM or SRM
        };
    };
    template<> struct attribute_values< test_ionization_polarity > {
        static constexpr const std::pair< const char * const, test_ionization_polarity > values [] = {
            { "Positive Polarity", Positive_Polarity }
            , {"Negative Polarity", Negative_Polarity }
        };
    };
    template<> struct attribute_values< experiment_type > {
        static constexpr const std::pair< const char * const, experiment_type > values [] = {
            { "Centroid Mass Spectrum", Centroid_Mass_Spectrum }
            , {"Profile Mass Spectrum", Profile_Mass_Spectrum }
        };
    };

    // ----------------- //
    template< typename T > struct find_attribute {
        std::optional< T > operator()( const std::string_view& view ) const {
            for ( auto& a: attribute_values< T >::values ) {
                if ( view.compare( a.first ) == 0 )
                    return a.second;
            }
            return {};
        }
    };

    class AndiMS::impl {
    public:
        impl() : isCounting_{ false, false }, jobj_{} {
        }
        std::array< bool, 2 > isCounting_; // tic,intensity_values
        boost::json::object jobj_;
        std::vector< data_tuple > data_;
        std::bitset< data_tuple_size > data_state_;

        std::vector< int32_t > intensities_;
        std::vector< double > masses_;
        std::map< int, std::pair< double, std::vector< int32_t > > > transformed_;

        std::optional< test_ionization_polarity > ion_polarity_;
        std::optional< test_scan_function > scan_function_;
        std::optional< experiment_type > is_centroid_;
        std::chrono::time_point< std::chrono::system_clock, std::chrono::nanoseconds> tp_inject_;
        std::shared_ptr< DataReader > dataReader_;
        std::mutex mutex_;

        // ANDI/MS
        template< data_name_t N, typename T >
        void import_t( const std::vector<T>& data ) {
            if ( data_.empty() ) {
                data_.resize( data.size() );
                std::fill( data_.begin(), data_.end(), data_tuple{});
                data_state_ = {0};
            }
            auto it = data_.begin();
            for ( const auto& v: data )
                std::get< N >(*it++) = v;
            data_state_[ N ] = true;
        }

        void import_intensities( const std::vector<int32_t>& data ) {
            intensities_ = data;
        }
        void import_masses( const std::vector<double>& data ) {
            masses_ = data;
        }

        template<typename T>
        void import( const std::string& key, const std::vector< T >& data ) {
            if ( key == "a_d_coaddition_factor" ) { // int16_t
                import_t< a_d_coaddition_factor >( data );
            } else if ( key == "scan_index" ) { // int32_t
                import_t< scan_index >( data );
            } else if ( key == "point_count" ) {
                import_t< point_count >( data );
            } else if ( key == "flag_count" ) {
                import_t< flag_count >( data );
            } else if ( key == "actual_scan_number" ) {
                import_t< actual_scan_number >( data );
            } else if ( key == "a_d_sampling_rate" ) { // double
                import_t< a_d_sampling_rate >( data );
            } else if ( key == "scan_acquisition_time" ) {
                import_t< scan_acquisition_time >( data );
            } else if ( key == "scan_duration" ) {
                import_t< scan_duration >( data );
            } else if ( key == "inter_scan_time" ) {
                import_t< inter_scan_time >( data );
            } else if ( key == "resolution" ) {
                import_t< resolution >( data );
            } else if ( key == "total_intensity" ) {
                import_t< total_intensity >( data );
            } else if ( key == "mass_range_min" ) {
                import_t< mass_range_min >( data );
            } else if ( key == "mass_range_max" ) {
                import_t< mass_range_max >( data );
            } else if ( key == "time_range_min" ) {
                import_t< time_range_min >( data );
            } else if ( key == "time_range_max" ) {
                import_t< time_range_max >( data );
            }
        }

        void import( const std::string& key, const std::vector< std::string >& data ) {
        }

        std::vector< std::shared_ptr< adcontrols::Chromatogram > >
        close() {
            const auto& global_attributes = jobj_[ "global_attributes" ];

            boost::system::error_code ec;
            if ( auto jtp = global_attributes.find_pointer( "/experiment_date_time_stamp", ec ) ) {
                if ( jtp->is_string() )
                    tp_inject_ = time_stamp_parser{}( std::string( jtp->as_string() ), true );
            }

            if ( auto value = global_attributes.find_pointer( "/test_ionization_polarity", ec ) ) {
                ion_polarity_ = find_attribute< test_ionization_polarity >{}( value->as_string() );
            }
            if ( auto value = global_attributes.find_pointer( "/test_scan_function", ec ) ) {
                scan_function_ = find_attribute< test_scan_function >{}( value->as_string() );
            }
            if ( auto value = global_attributes.find_pointer( "/experiment_type", ec ) ) {
                is_centroid_ = find_attribute< experiment_type >{}( value->as_string() );
            }

            std::vector< std::shared_ptr< adcontrols::Chromatogram > > results;

            if ( auto tic = std::make_shared< adcontrols::Chromatogram >() ) {
                getTIC( *tic );
                results.emplace_back( std::move( tic ) );
            }

            transformed_ = transform(intensities_, masses_, data_ );

            // if ( scan_function_ && *scan_function_ == Selected_Ion_Detection ) {
            do {
                std::string polarity;
                for ( const auto& [ch,data]: transformed_ ) {
                    const auto& [mass,values] = data;
                    auto chro = std::make_shared< adcontrols::Chromatogram >();
                    chro->resize( data_.size() );
                    for ( size_t i = 0; i < chro->size(); ++i ) {
                        chro->setDatum( i, { std::get< scan_acquisition_time >( data_[i] ), values[i] } );
                    }
                    chro->setMinimumTime( std::get< scan_acquisition_time > ( data_.front() ) );
                    chro->setMaximumTime( std::get< scan_acquisition_time > ( data_.back() ) );
                    chro->setIsConstantSampledData( false );

                    auto pol = ( ion_polarity_ ?
                                 *ion_polarity_ == Negative_Polarity ? "neg" :
                                 *ion_polarity_ == Positive_Polarity ? "pos" : "" : "");
                    chro->addDescription( { "Create"
                            , (boost::format("%d, m/z %.2f %s") % ch % mass % pol).str()} );
                    chro->set_display_name( (boost::format("%d, m/z %.2f %s") % ch % mass % pol).str() );
                    auto data_attribute =
                        boost::json::value{{ "data_attribute", { "ion_polarity", pol }, { "mass", mass }, { "channel", ch }}};
                    chro->addDescription( { "__global_attributes", boost::json::serialize( jobj_[ "global_attributes"] ) } );
                    chro->addDescription( { "__data_attribute", boost::json::serialize( data_attribute ) } );
                    chro->set_time_of_injection_iso8601( iso8601{}( tp_inject_ ) );
                    chro->setIsCounting( isCounting_[ 1 ] );
                    if ( isCounting_[1] )
                        chro->setAxisLabel( adcontrols::plot::yAxis, "Intensity (counts)" );
                    results.emplace_back( std::move( chro ) );
                }
            } while ( 0 );
            return results;
        }

        void set_local_attribute( const std::string& var, const std::string& attr, const std::string& value ) {
            if ( attr == "units" && value == "Total Counts" ) {
                if ( var == "total_intensity" )
                    isCounting_[0] = true;
                else if ( var == "intensity_values" )
                    isCounting_[1] = true;
            }
        }

        void getTIC( adcontrols::Chromatogram& tic ) {
            ADDEBUG() << "##### \tgetTIC";
            tic.resize( data_.size() );
            for ( size_t i = 0; i < data_.size(); ++i ) {
                tic.setTime( i, std::get< scan_acquisition_time > ( data_[i] ) );
                tic.setIntensity( i, std::get< total_intensity > ( data_[i] ) );
            }
            tic.setMinimumTime( std::get< scan_acquisition_time > ( data_.front() ) );
            tic.setMaximumTime( std::get< scan_acquisition_time > ( data_.back() ) );
            tic.addDescription( { "Create", (boost::format("TIC/TIC.%d") % 1 ).str()  } );
            tic.addDescription( { "__global_attributes", boost::json::serialize( jobj_[ "global_attributes"] ) } );
            tic.setIsCounting( isCounting_[ 0 ] );
            tic.set_time_of_injection_iso8601( iso8601{}( tp_inject_ ) );
            if ( isCounting_[0] )
                tic.setAxisLabel( adcontrols::plot::yAxis, "Intensity (counts)" );
        }

        static std::map< int, std::pair< double, std::vector< int32_t > > >
        transform( const std::vector< int32_t >& intensities
                   , const std::vector< double >& masses
                   , const std::vector< data_tuple >& data ) {

            std::map< int, std::pair< double, std::vector< int32_t > > > map;
            size_t nChannels = intensities.size() / data.size();

            for ( size_t j = 0; j < nChannels; ++j )
                map[j].first = masses[ j ];

            for ( size_t i = 0; i < data.size(); ++i ) {
                for ( size_t j = 0; j < nChannels; ++j )
                    map[ j ].second.emplace_back( intensities[ i * nChannels + j ] );
            }
            return map;
        }

        std::shared_ptr< adcontrols::DataReader > dataReader( const AndiMS * p ) {
            if ( not dataReader_ ) {
                std::unique_lock lock( mutex_ );
                if ( not dataReader_ )
                    dataReader_ = std::make_shared< DataReader >( "AndiMS", p->shared_from_this() );
            }
            return dataReader_;
        }

    };

}

using namespace adnetcdf;

AndiMS::AndiMS() : impl_( std::make_unique<  impl >() )
{
}

AndiMS::~AndiMS()
{
}

std::vector< std::shared_ptr< adcontrols::Chromatogram > >
AndiMS::import( const nc::ncfile& file ) const
{
    namespace nc = adnetcdf::netcdf;

    auto att_ovld = overloaded{
        [&]( const std::string& data, const nc::attribute& att )->std::pair<std::string, std::string>{ return {data, att.name()}; },
        [&]( const auto& data, const nc::attribute& att )->std::pair<std::string, std::string>{ return { {}, att.name()}; },
    };

    auto var_ovld = overloaded{
        [&]( nc::null_datum_t, const nc::variable& var ) {},
        [&]( const auto& data, const nc::variable& var ) {
            ADDEBUG() << "\t\t" << var.name() << ", " << typeid(data).name() << "\t= [" << data.size() << "] unhandled data";
        },
        [&]( const std::vector< int16_t >& data, const nc::variable& var ) {
            impl_->import( var.name(), data );
        },
        [&]( const std::vector< int32_t >& data, const nc::variable& var ) {
            ( var.name() == "intensity_values" ) ? impl_->import_intensities( data ) : impl_->import( var.name(), data );
        },
        [&]( const std::vector< double >& data, const nc::variable& var ) {
            ( var.name() == "mass_values" ) ? impl_->import_masses( data ) : impl_->import( var.name(), data );
        },
        [&]( const std::vector< std::string >& data, const nc::variable& var ) {
            impl_->import( var.name(), data );
        }
    };

    do {
        boost::json::object ja;
        for ( const auto& att: file.atts() ) {
            auto [value,key] = std::visit( att_ovld, file.readData( att ), std::variant< nc::attribute >( att ) );
            ja[ key ] = value;
        }
        impl_->jobj_[ "global_attributes" ] = std::move(ja);
    } while ( 0 );

    boost::json::object jdata;

    for ( const auto& var: file.vars() ) {
        boost::json::object jv;
        if ( std::get< nc::variable::_natts >( var.value() ) > 0 ) {
            boost::json::array ja; // local_attribute
            for ( const auto& att: file.atts( var ) ) {
                auto [value,key] = std::visit( att_ovld, file.readData( att ), std::variant< nc::attribute >( att ) );
                impl_->set_local_attribute( var.name(), key, value );
                ja.emplace_back( boost::json::object{{key,value}} );
            }
            jv["atts"] = std::move( ja );
        }
        auto datum = file.readData( var );
        std::visit( var_ovld, datum, std::variant< nc::variable >(var) );

        // add data to json
        std::visit( overloaded{
                []( const nc::null_datum_t& ) {}
                    , [&]( const auto& arg ) { jv["size"] = arg.size(); }
                    , [&](const std::string& arg ) { jv["value"] = arg; }
                    , [&](const std::vector< std::string >& arg ) {
                        jv["values"] = boost::json::value_from(arg); }
                    }, datum );
        jdata[ var.name() ] = std::move( jv );
    }

    impl_->jobj_[ "data" ] = std::move( jdata );

    // ADDEBUG() << impl_->jobj_ << std::endl;

    return impl_->close();
}

std::optional< std::string >
AndiMS::find_global_attribute( const std::string& attribute ) const
{
    if ( impl_->jobj_.if_contains( "global_attributes" ) ) {
        const auto& a= impl_->jobj_.at( "global_attributes" );
        boost::system::error_code ec;
        if ( auto value = a.find_pointer( attribute, ec ) ) {
            return std::string( value->as_string() );
        }
    }
    return {};
}

const boost::json::object&
AndiMS::json() const
{
    return impl_->jobj_;
}

bool
AndiMS::has_spectra() const
{
    return impl_->scan_function_ && *impl_->scan_function_ == Mass_Scan;
}

const std::vector< data_tuple >&
AndiMS::data() const
{
    return impl_->data_;
}

const std::map< int, std::pair< double, std::vector< int32_t > > >&
AndiMS::transformed() const
{
    return impl_->transformed_;
}

////////////////////////////

size_t
AndiMS::getFunctionCount() const
{
    // ADDEBUG() << "================ " << __FUNCTION__ << " ==================";
    return 1;
}

size_t
AndiMS::getSpectrumCount( int fcn ) const
{
    // ADDEBUG() << "================ " << __FUNCTION__ << " ================== fcn: " << fcn;
    return impl_->data_.size();
}

size_t
AndiMS::getChromatogramCount() const
{
    // ADDEBUG() << "================ " << __FUNCTION__ << " ==================";
    return 0;
}

bool
AndiMS::getTIC( int fcn, adcontrols::Chromatogram& tic ) const
{
    // ADDEBUG() << "================ " << __FUNCTION__ << " ==================";
    impl_->getTIC( tic );
    return true;
}

bool
AndiMS::getSpectrum( int fcn, size_t pos, adcontrols::MassSpectrum& ms, uint32_t objid ) const
{
    // ADDEBUG() << "================ " << __FUNCTION__ << " ================== objid: " << objid;
    return false;
}

size_t
AndiMS::posFromTime( double arg ) const
{
    // ADDEBUG() << "================ " << __FUNCTION__ << " ==================: " << arg;
    return 0;
}

double
AndiMS::timeFromPos( size_t arg ) const
{
    // ADDEBUG() << "================ " << __FUNCTION__ << " ================== " << arg;
    return 0;
}

bool
AndiMS::getChromatograms( const std::vector< std::tuple<int, double, double> >&
                       , std::vector< adcontrols::Chromatogram >&
                       , std::function< bool (long curr, long total ) > progress
                       , int /* begPos */
                       , int /* endPos */ ) const
{
    // ADDEBUG() << "================ " << __FUNCTION__ << " ==================";
    return false;
}

size_t
AndiMS::dataReaderCount() const
{
    // ADDEBUG() << "================ " << __FUNCTION__ << " ==================";
    return 1;
}

const adcontrols::DataReader *
AndiMS::dataReader( size_t idx ) const
{
    // ADDEBUG() << "================ " << __FUNCTION__ << " ==================: " << idx;
    if ( idx == 0 )
        return impl_->dataReader( this ).get();
    return nullptr;
}

const adcontrols::DataReader *
AndiMS::dataReader( const boost::uuids::uuid& uuid ) const
{
    // ADDEBUG() << "================ " << __FUNCTION__ << " ==================: " << uuid;
    return impl_->dataReader( this ).get();
}

std::vector < std::shared_ptr< adcontrols::DataReader > >
AndiMS::dataReaders( bool allPossible ) const
{
    try {
        return std::vector < std::shared_ptr< adcontrols::DataReader > >{ impl_->dataReader( this ) };
    } catch ( std::exception& ex ) {
        ADDEBUG() << "## Exception: " << ex.what();
    }
    return {};
}
