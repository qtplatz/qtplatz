// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC
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

#include "andichromatogram.hpp"
#include "description.hpp"
#include "ncfile.hpp"
#include "attribute.hpp"
#include "variable.hpp"
#include <__concepts/copyable.h>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/baseline.hpp>
#include <adcontrols/baselines.hpp>
#include <adcontrols/peak.hpp>
#include <adcontrols/peaks.hpp>
#include <adportable/debug.hpp>
#include <algorithm>
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

    class AndiChromatogram::impl {
    public:
        std::map< std::string, std::string > global_attribures_;
        adcontrols::Peaks peaks_;
        adcontrols::Baselines baselines_;
        std::shared_ptr< adcontrols::Chromatogram > chro_;

        void adjust_peak_size( size_t size ) {
            if ( size != peaks_.size() )
                static_cast< adcontrols::Peaks::vector_type& >(peaks_).resize( size );
        }
        void adjust_baseline_size( size_t size ) {
            if ( size != baselines_.size() )
                static_cast< adcontrols::Baselines::vector_type& >(baselines_).resize( size );
        }

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

        void set_local_attribute( const std::string& var, const std::string& attr, const std::string& value ) {
            if ( var == "ordinate_values" ) {
                if ( attr == "uniform_sampling_flag" )
                    chro_->setIsCounting( value == "Y" );
                if ( attr == "autosampler_position" )
                    ;
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
        }

    };

}

using namespace adnetcdf;

AndiChromatogram::AndiChromatogram() : impl_( new impl() )
{
}

AndiChromatogram::~AndiChromatogram()
{
}

std::shared_ptr< adcontrols::Chromatogram >
AndiChromatogram::import( const nc::ncfile& file ) const
{
    namespace nc = adnetcdf::netcdf;

    impl_->chro_ = std::make_shared< adcontrols::Chromatogram >();

    auto att_ovld = overloaded{
        [&]( const std::string& data, const nc::attribute& att )->std::pair<std::string, std::string>{ return {data, att.name()}; },
        [&]( const auto& data, const nc::attribute& att )->std::pair<std::string, std::string>{ return { {}, att.name()}; },
    };

    auto var_ovld = overloaded{
        [&]( nc::null_datum_t, const nc::variable& var ) {},
        [&]( const auto& data, const nc::variable& var ) {
            ADDEBUG() << "\t\t" << var.name() << "\t= [" << data.size() << "] unhandled data";
        },
        [&]( const std::vector< float >& data, const nc::variable& var ) {
            impl_->import( var.name(), data );
        },
        [&]( const std::vector< std::string >& data, const nc::variable& var ) {
            ADDEBUG() << "\t\t" << var.name();
        }
    };

    for ( const auto& att: file.atts() ) {
        auto data = std::visit( att_ovld, file.readData( att ), std::variant< nc::attribute >( att ) );
        impl_->chro_->addDescription( adcontrols::description( { data.second, data.first } ) );
        impl_->global_attribures_[ data.second ] = data.first;
    }

    for ( const auto& var: file.vars() ) {
        if ( std::get< nc::variable::_natts >( var.value() ) > 0 ) {
            for ( const auto& att: file.atts( var ) ) {
                auto data = std::visit( att_ovld, file.readData( att ), std::variant< nc::attribute >( att ) );
                impl_->set_local_attribute( var.name(), data.second, data.first );
            }
        }
        auto datum = file.readData( var );
        std::visit( var_ovld, datum, std::variant< nc::variable >(var) );
    }
    impl_->close();

    return impl_->chro_;
}


std::optional< std::string >
AndiChromatogram::dataset_completeness() const
{
    auto it =  impl_->global_attribures_.find( "dataset_completeness" );
    if ( it != impl_->global_attribures_.end() )
        return it->second;
    return {};
}

std::optional< std::string > AndiChromatogram::aia_template_revision() const
{
    auto it =  impl_->global_attribures_.find( "aia_template_revision" );
    if ( it != impl_->global_attribures_.end() )
        return it->second;
    return {};

}

std::optional< std::string > AndiChromatogram::netcdf_revision() const
{
    auto it =  impl_->global_attribures_.find( "netcdf_revision" );
    if ( it != impl_->global_attribures_.end() )
        return it->second;
    return {};

}

std::optional< std::string > AndiChromatogram::languages() const // = "English"
{
    auto it =  impl_->global_attribures_.find( "languages" );
    if ( it != impl_->global_attribures_.end() )
        return it->second;
    return {};

}

std::optional< std::string > AndiChromatogram::administrative_comments() const // = ""
{
    auto it =  impl_->global_attribures_.find( "administrative_comments" );
    if ( it != impl_->global_attribures_.end() )
        return it->second;
    return {};

}

std::optional< std::string > AndiChromatogram::dataset_origin() const // = "Shimadzu Corporation"
{

    auto it =  impl_->global_attribures_.find( "dataset_origin" );
    if ( it != impl_->global_attribures_.end() )
        return it->second;
    return {};
}

std::optional< std::string > AndiChromatogram::dataset_owner() const //  = ""
{
    auto it =  impl_->global_attribures_.find( "dataset_owner" );
    if ( it != impl_->global_attribures_.end() )
        return it->second;
    return {};

}

std::optional< std::string > AndiChromatogram::dataset_date_time_stamp() const // = "20230831150758+0900"
{
    auto it =  impl_->global_attribures_.find( "dataset_date_time_stamp" );
    if ( it != impl_->global_attribures_.end() )
        return it->second;
    return {};

}

std::optional< std::string > AndiChromatogram::injection_date_time_stamp() const //   = "20230831150758+0900"
{
    auto it =  impl_->global_attribures_.find( "injection_date_time_stamp" );
    if ( it != impl_->global_attribures_.end() )
        return it->second;
    return {};
}

std::optional< std::string > AndiChromatogram::experiment_title() const // = ""
{
    auto it =  impl_->global_attribures_.find( "experiment_title" );
    if ( it != impl_->global_attribures_.end() )
        return it->second;
    return {};
}

std::optional< std::string > AndiChromatogram::operator_name() const // = "System Administrator"
{
    auto it =  impl_->global_attribures_.find( "operator_name" );
    if ( it != impl_->global_attribures_.end() )
        return it->second;
    return {};
}

std::optional< std::string > AndiChromatogram::separation_experiment_type() const // = ""
{
    auto it =  impl_->global_attribures_.find( "separation_experiment_type" );
    if ( it != impl_->global_attribures_.end() )
        return it->second;
    return {};
}

std::optional< std::string > AndiChromatogram::company_method_name() const // = ""
{
    auto it =  impl_->global_attribures_.find( "company_method_name" );
    if ( it != impl_->global_attribures_.end() )
        return it->second;
    return {};
}

std::optional< std::string > AndiChromatogram::company_method_id() const // = ""
{
    auto it =  impl_->global_attribures_.find( "company_method_id" );
    if ( it != impl_->global_attribures_.end() )
        return it->second;
    return {};
}

std::optional< std::string > AndiChromatogram::pre_experiment_program_name() const // = ""
{
    auto it =  impl_->global_attribures_.find( "pre_experiment_program_name" );
    if ( it != impl_->global_attribures_.end() )
        return it->second;
    return {};
}

std::optional< std::string > AndiChromatogram::post_experiment_program_name() const // = ""
{
    auto it =  impl_->global_attribures_.find( "post_experiment_program_name" );
    if ( it != impl_->global_attribures_.end() )
        return it->second;
    return {};
}

std::optional< std::string > AndiChromatogram::source_file_reference() const // = "D:\\????\\COR40_1.lcd"
{
    auto it =  impl_->global_attribures_.find( "source_file_reference" );
    if ( it != impl_->global_attribures_.end() )
        return it->second;
    return {};
}

std::optional< std::string > AndiChromatogram::sample_id_comments() const // = ""
{
    auto it =  impl_->global_attribures_.find( "sample_id_comments" );
    if ( it != impl_->global_attribures_.end() )
        return it->second;
    return {};
}

std::optional< std::string > AndiChromatogram::sample_id() const // = ""
{
    auto it =  impl_->global_attribures_.find( "sample_id" );
    if ( it != impl_->global_attribures_.end() )
        return it->second;
    return {};
}

std::optional< std::string > AndiChromatogram::sample_name() const // = "CORx40"
{
    auto it =  impl_->global_attribures_.find( "sample_name" );
    if ( it != impl_->global_attribures_.end() )
        return it->second;
    return {};
}

std::optional< std::string > AndiChromatogram::sample_type() const // = "Unknown"
{
    auto it =  impl_->global_attribures_.find( "sample_type" );
    if ( it != impl_->global_attribures_.end() )
        return it->second;
    return {};
}

std::optional< std::string > AndiChromatogram::sample_injection_volume() const // = 1.f
{
    auto it =  impl_->global_attribures_.find( "sample_injection_volume" );
    if ( it != impl_->global_attribures_.end() )
        return it->second;
    return {};
}

std::optional< std::string > AndiChromatogram::sample_amount() const // = 1.f
{
    auto it =  impl_->global_attribures_.find( "sample_amount" );
    if ( it != impl_->global_attribures_.end() )
        return it->second;
    return {};
}

std::optional< std::string > AndiChromatogram::detection_method_table_name() const // = ""
{
    auto it =  impl_->global_attribures_.find( "detection_method_table_name" );
    if ( it != impl_->global_attribures_.end() )
        return it->second;
    return {};
}

std::optional< std::string > AndiChromatogram::detection_method_comments() const // = ""
{
    auto it =  impl_->global_attribures_.find( "detection_method_comments" );
    if ( it != impl_->global_attribures_.end() )
        return it->second;
    return {};
}

std::optional< std::string > AndiChromatogram::detection_method_name() const // = "D:\\????\\COR.lcm"
{
    auto it =  impl_->global_attribures_.find( "detection_method_name" );
    if ( it != impl_->global_attribures_.end() )
        return it->second;
    return {};
}

std::optional< std::string > AndiChromatogram::detector_name() const // = "???o??A"
{
    auto it =  impl_->global_attribures_.find( "detector_name" );
    if ( it != impl_->global_attribures_.end() )
        return it->second;
    return {};
}

std::optional< std::string > AndiChromatogram::detector_unit() const // = "Volts"
{
    auto it =  impl_->global_attribures_.find( "detector_unit" );
    if ( it != impl_->global_attribures_.end() )
        return it->second;
    return {};
}

std::optional< std::string > AndiChromatogram::raw_data_table_name() const // = "D:\\????\\COR40_1.lcd"
{
    auto it =  impl_->global_attribures_.find( "raw_data_table_name" );
    if ( it != impl_->global_attribures_.end() )
        return it->second;
    return {};
}

std::optional< std::string > AndiChromatogram::retention_unit() const // = "Seconds"
{
    auto it =  impl_->global_attribures_.find( "retention_unit" );
    if ( it != impl_->global_attribures_.end() )
        return it->second;
    return {};
}

std::optional< std::string > AndiChromatogram::peak_processing_results_table_name() const // = ""
{
    auto it =  impl_->global_attribures_.find( "peak_processing_results_table_name" );
    if ( it != impl_->global_attribures_.end() )
        return it->second;
    return {};
}

std::optional< std::string > AndiChromatogram::peak_processing_results_comments() const // = ""
{
    auto it =  impl_->global_attribures_.find( "peak_processing_results_comments" );
    if ( it != impl_->global_attribures_.end() )
        return it->second;
    return {};
}

std::optional< std::string > AndiChromatogram::peak_processing_method_name() const // = "D:\\????\\COR.lcm"
{
    auto it =  impl_->global_attribures_.find( "peak_processing_method_name" );
    if ( it != impl_->global_attribures_.end() )
        return it->second;
    return {};
}

std::optional< std::string > AndiChromatogram::peak_processing_date_time_stamp() const // = ""
{
    auto it =  impl_->global_attribures_.find( "peak_processing_date_time_stamp" );
    if ( it != impl_->global_attribures_.end() )
        return it->second;
    return {};
}
