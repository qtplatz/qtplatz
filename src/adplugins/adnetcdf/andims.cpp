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
#include <algorithm>
#include <bitset>
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

    // ANDI/MS attributes
    enum data_name_t {
        a_d_sampling_rate          // 0
        , scan_acquisition_time    // 1 time
        , scan_duration            // 2
        , inter_scan_time          // 3
        , resolution               // 4
        , total_intensity          // 5
        , mass_range_min           // 6
        , mass_range_max           // 7
        , time_range_min           // 8
        , time_range_max           // 9
        , a_d_coaddition_factor    // 10 int16_t
        , scan_index               // 11 int32_t
        , point_count              // 12 int32_t
        , flag_count               // 13 int32_t
        , actual_scan_number       // 14 int32_t
        , data_tuple_size
    };

    // ANDI/MS aqttribute types
    typedef std::tuple< double   // 0 a_d_sampling_rate
                        , double // 1 scan_acquisition_time
                        , double // 2 scan_duration
                        , double // 3 inter_scan_time
                        , double // 4 resolution
                        , double // 5 total_intensity
                        , double // 6 mass_range_min
                        , double // 7 mass_range_max
                        , double // 8 time_range_min
                        , double // 9 time_range_max
                        , int16_t // 10 a_d_coaddition_factor // int16_t
                        , int32_t // 11 scan_index            // int32_t
                        , int32_t // 12 point_count           // int32_t
                        , int32_t // 13 flag_count            // int32_t
                        , int32_t // 14 actual_scan_number    // int32_t
                        > data_tuple;

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
            std::string tp_inject;
            boost::system::error_code ec;
            if ( auto jtp = jobj_[ "global_attribures" ].find_pointer( "experiment_date_time_stamp", ec ) ) {
                if ( jtp->is_string() )
                    tp_inject = iso8601{}( time_stamp_parser{}( std::string( jtp->as_string() ), true ) );
            }

            std::vector< std::shared_ptr< adcontrols::Chromatogram > > results;

            if ( auto tic = std::make_shared< adcontrols::Chromatogram >() ) {
                tic->resize( data_.size() );
                for ( size_t i = 0; i < data_.size(); ++i ) {
                    tic->setTime( i, std::get< scan_acquisition_time > ( data_[i] ) );
                    tic->setIntensity( i, std::get< total_intensity > ( data_[i] ) );
                }
                tic->minimumTime( std::get< scan_acquisition_time > ( data_.front() ) );
                tic->maximumTime( std::get< scan_acquisition_time > ( data_.back() ) );
                tic->addDescription( { "Create", "TIC" } );
                tic->addDescription( { "__global_attributes", boost::json::serialize( jobj_[ "global_attributes"] ) } );
                tic->setIsCounting( isCounting_[ 0 ] );
                tic->set_time_of_injection_iso8601( tp_inject );
                if ( isCounting_[0] )
                    tic->setAxisLabel( adcontrols::plot::yAxis, "Intensity (counts)" );
                results.emplace_back( std::move( tic ) );
            }


            std::map< int, std::vector< int32_t > > ordinate_values;
            if ( data_state_.all() && ( masses_.size() == intensities_.size() ) ){
                size_t nChannels = intensities_.size() / data_.size();
                std::set< int > masses;
                for ( const auto& mass: masses_ )
                    masses.insert( mass * 1000 );

                if ( masses.size() != nChannels ) {
                    ADDEBUG() << "## Number of mass,intensity pair does not match, nChannels = " << nChannels << ", # of masses" << masses.size();
                }

                // for ( size_t i = 0; i < masses_.size(); ++i ) {
                //     const int key = masses_[i] * 1000;
                //     ordinate_values[ key ].emplace_back( intensities_[ i ] );
                // }

                for ( size_t i = 0; i < data_.size(); ++i ) {
                    for ( size_t j = 0; j < nChannels; ++j ) {
                        ordinate_values[ j ].emplace_back( intensities_[ i * nChannels + j ] );
                    }
                }
            }

            for ( const auto& [ch,values]: ordinate_values ) {
                auto chro = std::make_shared< adcontrols::Chromatogram >();
                chro->resize( data_.size() );
                for ( size_t i = 0; i < chro->size(); ++i ) {
                    chro->setTime( i, std::get< scan_acquisition_time >( data_[i] ) );
                    chro->setIntensity( i, values[ i ] );
                }
                chro->minimumTime( std::get< scan_acquisition_time > ( data_.front() ) );
                chro->maximumTime( std::get< scan_acquisition_time > ( data_.back() ) );
                chro->addDescription( { "Create", (boost::format("m/z %.2f") % masses_[ch]).str()} );
                chro->addDescription( { "__global_attributes", boost::json::serialize( jobj_[ "global_attributes"] ) } );
                chro->set_time_of_injection_iso8601( tp_inject );
                chro->setIsCounting( isCounting_[ 1 ] );
                if ( isCounting_[1] )
                    chro->setAxisLabel( adcontrols::plot::yAxis, "Intensity (counts)" );
                results.emplace_back( std::move( chro ) );
            }
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
        impl_->jobj_[ "global_attributes"] = std::move(ja);
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
