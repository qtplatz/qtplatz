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

#include "andichromatography.hpp"
#include "ncfile.hpp"
#include "attribute.hpp"
#include "variable.hpp"
#include "timestamp.hpp"
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/baseline.hpp>
#include <adcontrols/baselines.hpp>
#include <adcontrols/peak.hpp>
#include <adcontrols/peaks.hpp>
#include <adportable/debug.hpp>
#include <adportable/json_helper.hpp>
#include <boost/format.hpp>
#include <boost/json.hpp>
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

    class AndiChromatography::impl {
    public:
        impl() : chro_( std::make_shared< adcontrols::Chromatogram >() )
               , jobj_{} {}
        boost::json::object jobj_;
        adcontrols::Peaks peaks_;
        adcontrols::Baselines baselines_;
        std::shared_ptr< adcontrols::Chromatogram > chro_;

        // ANDI/Chromatography
        void adjust_peak_size( size_t size ) {
            if ( size != peaks_.size() )
                static_cast< adcontrols::Peaks::vector_type& >(peaks_).resize( size );
        }

        // ANDI/Chromatography
        void adjust_baseline_size( size_t size ) {
            if ( size != baselines_.size() )
                static_cast< adcontrols::Baselines::vector_type& >(baselines_).resize( size );
        }

        // ANDI/Chromatography
        bool set_peak( const std::string& key, const std::vector< float >& data ) {
            auto& pks = static_cast< adcontrols::Peaks::vector_type& >(peaks_);
            adjust_peak_size( data.size() );

            if ( key == "peak_retention_time" ) {
                for ( size_t i = 0; i < data.size(); ++i )
                    pks[i].setPeakTime( data[i] );
                return true;
            } else if ( key == "peak_amount" ) {
                for ( size_t i = 0; i < data.size(); ++i )
                    pks[i].setPeakAmount( data[i] );
                return true;
            } else if ( key == "peak_start_time" ) {
                for ( size_t i = 0; i < data.size(); ++i )
                    pks[i].setStartTime( data[i] );
                return true;
            } else if ( key == "peak_end_time" ) {
                for ( size_t i = 0; i < data.size(); ++i )
                    pks[i].setEndTime( data[i] );
                return true;
            } else if ( key == "peak_width" ) {
                for ( size_t i = 0; i < data.size(); ++i )
                    pks[i].setPeakWidth( data[i] );
                return true;
            } else if ( key == "peak_area" ) {
                for ( size_t i = 0; i < data.size(); ++i )
                    pks[i].setPeakArea( data[i] );
                return true;
            } else if ( key == "peak_area_percent" ) {
                for ( size_t i = 0; i < data.size(); ++i )
                    pks[i].setPercentArea( data[i] );
                return true;
            } else if ( key == "peak_height" ) {
                for ( size_t i = 0; i < data.size(); ++i )
                    pks[i].setPeakHeight( data[i] );
                return true;
            } else if ( key == "peak_height_percent" ) {
                for ( size_t i = 0; i < data.size(); ++i )
                    pks[i].setPercentHeight( data[i] );
                return true;
            } else if ( key == "retention_index" ) {
                return true;
            } else if ( key == "migration_time" ) {
                return true;
            } else if ( key == "peak_asymmetry" ) {
                return true;
            } else if ( key == "peak_efficiency" ) {
                return true;
            } else if ( key == "mass_on_column" ) {
                return true;
            }
            return false;
        }

        // ANDI/Chromatography
        bool set_baseline( const std::string& key, const std::vector< float >& data ) {
            adjust_baseline_size( data.size() );
            auto& bss = static_cast< adcontrols::Baselines::vector_type& >(baselines_);

            if ( key == "baseline_start_time" ) {
                for ( size_t i = 0; i < data.size(); ++i )
                    bss[i].setStartTime( data[i] );
                return true;
            } else if ( key == "baseline_stop_time" ) {
                for ( size_t i = 0; i < data.size(); ++i )
                    bss[i].setStopTime( data[i] );
                return true;
            } else if ( key == "baseline_stop_value" ) {
                for ( size_t i = 0; i < data.size(); ++i )
                    bss[i].setStopHeight( data[i] );
                return true;
            }
            return false;
        }

        // ANDI/Chromatography
        void set_local_attribute( const std::string& var, const std::string& attr, const std::string& value ) {
            if ( var == "ordinate_values" ) {
                if ( attr == "uniform_sampling_flag" )
                    chro_->setIsConstantSampledData( value == "Y" );
                if ( attr == "autosampler_position" )
                    ;
            }
        }

        // ANDI/Chromatography
        void import( const std::string& key, const std::vector< int16_t >& data ) {
            if ( key == "a_d_coaddition_factor" ) { // ANDI/MS
                // import_t< a_d_coaddition_factor >( data );
            } else {
                ADDEBUG() << "import std::vector<int16_t> key,size=" << std::make_pair(key, data.size()) << "\tunhandled";
            }
        }

        void import( const std::string& key, const std::vector< int32_t >& data ) {
            ADDEBUG() << "import std::vector<int32_t> key,size=" << std::make_pair(key, data.size()) << "\tunhandled";
        }

        // ANDI/Chromatography
        void import( const std::string& key, const std::vector< float >& data ) {
            if ( key == "ordinate_values" ) {
                chro_->setIntensityArray( data );
            } else if ( key == "detector_maximum_value" ) {
            } else if ( key == "detector_minimum_value" ) {
            } else if ( key == "actual_run_time_length" ) {
                chro_->maximumTime( data.at( 0 ) );
            } else if ( key == "actual_sampling_interval" ) {
                chro_->sampInterval( data.at( 0 ) );
            } else if ( key == "actual_delay_time" ) {
                chro_->minimumTime( data.at( 0 ) );
            } else {
                set_peak( key, data ) || set_baseline( key, data );
            }
        }

        void import( const std::string& key, const std::vector< std::string >& data ) {
            if ( key == "peak_name" ) {
            } else if ( key == "peak_start_detection_code" ) {
            } else if ( key == "peak_stop_detection_code" ) {
            } else if ( key == "manually_reintegrated_peaks" ) {
            }
        }

        void close_peaks() {
            for ( size_t id = 0; id < baselines_.size(); ++id )
                static_cast< adcontrols::Baselines::vector_type& >(baselines_)[id].setBaseId( id );
            for ( size_t id = 0; id < peaks_.size(); ++id ) {
                static_cast< adcontrols::Peaks::vector_type& >(peaks_)[id].setPeakId( id );
                static_cast< adcontrols::Peaks::vector_type& >(peaks_)[id].setBaseId( id );
            }
        }

        void close() {
            close_peaks();
            chro_->setPeaks( peaks_ );
            chro_->setBaselines( baselines_ );
            if ( not chro_->getTimeArray() ) {
                double sampIntval = chro_->sampInterval();
                double minTime = chro_->minimumTime();
                for ( size_t i = 0; i < chro_->size(); ++i )
                    chro_->setTime( i, minTime + sampIntval * i );
            }
            boost::system::error_code ec;
            if ( auto attr = adportable::json_helper::find_pointer( jobj_, "/global_attributes/sample_name", ec ) )
                chro_->addDescription( { "samplae_name", std::string( attr->as_string() ) } );

            if ( auto jtp = jobj_[ "global_attribures" ].find_pointer( "injection_date_time_stamp", ec ) ) {
                if ( jtp->is_string() ) {
                    auto tp = iso8601{}( time_stamp_parser{}( std::string( jtp->as_string() ), true ) );
                    chro_->set_time_of_injection_iso8601( tp );
                }
            }


        }

    };

}

using namespace adnetcdf;

AndiChromatography::AndiChromatography() : impl_( std::make_unique< impl >() )
{
}

AndiChromatography::~AndiChromatography()
{
}

std::vector< std::shared_ptr< adcontrols::Chromatogram > >
AndiChromatography::import( const nc::ncfile& file ) const
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
        [&]( const std::vector< int32_t >& data, const nc::variable& var ) {
            impl_->import( var.name(), data );
        },
        [&]( const std::vector< int16_t >& data, const nc::variable& var ) {
            impl_->import( var.name(), data );
        },
        [&]( const std::vector< float >& data, const nc::variable& var ) {
            impl_->import( var.name(), data );
        },
        [&]( const std::vector< std::string >& data, const nc::variable& var ) {
            impl_->import( var.name(), data );
        }
    };

    { // json --------->
        boost::json::object ja;
        for ( const auto& att: file.atts() ) {
            auto [value,key] = std::visit( att_ovld, file.readData( att ), std::variant< nc::attribute >( att ) );
            ja[ key ] = value;
        }
        impl_->jobj_["global_attributes"] = std::move(ja);
    } // <-------------

    boost::json::object jdata;
    for ( const auto& var: file.vars() ) {
        boost::json::object jv;
        if ( std::get< nc::variable::_natts >( var.value() ) > 0 ) {
            boost::json::array ja;
            for ( const auto& att: file.atts( var ) ) {
                auto [value,key] = std::visit( att_ovld, file.readData( att ), std::variant< nc::attribute >( att ) );
                impl_->set_local_attribute( var.name(), key, value );
                ja.emplace_back( boost::json::object{{key,value}} );
            }
            jv["atts"] = std::move( ja );
        }
        auto datum = file.readData( var );
        std::visit( var_ovld, datum, std::variant< nc::variable >(var) );

        { // add data to json ------------>
            std::visit( overloaded{
                    []( const nc::null_datum_t& ) {}
                        , [&]( const auto& arg ) { jv["size"] = arg.size(); }
                        , [&]( const std::vector< std::string >& arg ) {
                            jv["values"] = boost::json::value_from(arg); }
                        }, datum );
            jdata[ var.name() ] = jv;
        } // <------------
    }
    impl_->jobj_["data"] = std::move( jdata );

    // ADDEBUG() << impl_->jobj_ << std::endl;

    impl_->close();
    return std::vector< std::shared_ptr< adcontrols::Chromatogram > >{ impl_->chro_ };
}

std::optional< std::string >
AndiChromatography::find_global_attribute( const std::string& attribute ) const
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
AndiChromatography::json() const
{
    return impl_->jobj_;
}


std::optional< std::string >
AndiChromatography::dataset_completeness() const
{
    return find_global_attribute( "dataset_completeness" );
}

std::optional< std::string > AndiChromatography::aia_template_revision() const
{
    return find_global_attribute( "aia_template_revision" );
}

std::optional< std::string > AndiChromatography::netcdf_revision() const
{
    return find_global_attribute( "netcdf_revision" );
}

std::optional< std::string > AndiChromatography::languages() const // = "English"
{
    return find_global_attribute( "languages" );
}

std::optional< std::string > AndiChromatography::administrative_comments() const // = ""
{
    return find_global_attribute( "administrative_comments" );
}

std::optional< std::string > AndiChromatography::dataset_origin() const // = "Shimadzu Corporation"
{

    return find_global_attribute( "dataset_origin" );
}

std::optional< std::string > AndiChromatography::dataset_owner() const //  = ""
{
    return find_global_attribute( "dataset_owner" );
}

std::optional< std::string > AndiChromatography::dataset_date_time_stamp() const // = "20230831150758+0900"
{
    return find_global_attribute( "dataset_date_time_stamp" );

}

std::optional< std::string > AndiChromatography::injection_date_time_stamp() const //   = "20230831150758+0900"
{
    return find_global_attribute( "injection_date_time_stamp" );
}

std::optional< std::string > AndiChromatography::experiment_title() const // = ""
{
    return find_global_attribute( "experiment_title" );
}

std::optional< std::string > AndiChromatography::operator_name() const // = "System Administrator"
{
    return find_global_attribute( "operator_name" );
}

std::optional< std::string > AndiChromatography::separation_experiment_type() const // = ""
{
    return find_global_attribute( "separation_experiment_type" );
}

std::optional< std::string > AndiChromatography::company_method_name() const // = ""
{
    return find_global_attribute( "company_method_name" );
}

std::optional< std::string > AndiChromatography::company_method_id() const // = ""
{
    return find_global_attribute( "company_method_id" );
}

std::optional< std::string > AndiChromatography::pre_experiment_program_name() const // = ""
{
    return find_global_attribute( "pre_experiment_program_name" );
}

std::optional< std::string > AndiChromatography::post_experiment_program_name() const // = ""
{
    return find_global_attribute( "post_experiment_program_name" );
}

std::optional< std::string > AndiChromatography::source_file_reference() const // = "D:\\????\\COR40_1.lcd"
{
    return find_global_attribute( "source_file_reference" );
}

std::optional< std::string > AndiChromatography::sample_id_comments() const // = ""
{
    return find_global_attribute( "sample_id_comments" );
}

std::optional< std::string > AndiChromatography::sample_id() const // = ""
{
    return find_global_attribute( "sample_id" );
}

std::optional< std::string > AndiChromatography::sample_name() const // = "CORx40"
{
    return find_global_attribute( "sample_name" );
}

std::optional< std::string > AndiChromatography::sample_type() const // = "Unknown"
{
    return find_global_attribute( "sample_type" );
}

std::optional< std::string > AndiChromatography::sample_injection_volume() const // = 1.f
{
    return find_global_attribute( "sample_injection_volume" );
}

std::optional< std::string > AndiChromatography::sample_amount() const // = 1.f
{
    return find_global_attribute( "sample_amount" );
}

std::optional< std::string > AndiChromatography::detection_method_table_name() const // = ""
{
    return find_global_attribute( "detection_method_table_name" );
}

std::optional< std::string > AndiChromatography::detection_method_comments() const // = ""
{
    return find_global_attribute( "detection_method_comments" );
}

std::optional< std::string > AndiChromatography::detection_method_name() const // = "D:\\????\\COR.lcm"
{
    return find_global_attribute( "detection_method_name" );
}

std::optional< std::string > AndiChromatography::detector_name() const // = "???o??A"
{
    return find_global_attribute( "detector_name" );
}

std::optional< std::string > AndiChromatography::detector_unit() const // = "Volts"
{
    return find_global_attribute( "detector_unit" );
}

std::optional< std::string > AndiChromatography::raw_data_table_name() const // = "D:\\????\\COR40_1.lcd"
{
    return find_global_attribute( "raw_data_table_name" );
}

std::optional< std::string > AndiChromatography::retention_unit() const // = "Seconds"
{
    return find_global_attribute( "retention_unit" );
}

std::optional< std::string > AndiChromatography::peak_processing_results_table_name() const // = ""
{
    return find_global_attribute( "peak_processing_results_table_name" );
}

std::optional< std::string > AndiChromatography::peak_processing_results_comments() const // = ""
{
    return find_global_attribute( "peak_processing_results_comments" );
}

std::optional< std::string > AndiChromatography::peak_processing_method_name() const // = "D:\\????\\COR.lcm"
{
    return find_global_attribute( "peak_processing_method_name" );
}

std::optional< std::string > AndiChromatography::peak_processing_date_time_stamp() const // = ""
{
    return find_global_attribute( "peak_processing_date_time_stamp" );
}
