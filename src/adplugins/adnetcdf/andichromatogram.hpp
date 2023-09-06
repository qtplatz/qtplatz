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

#pragma once

#include <map>
#include <string>
#include <vector>
#include <memory>

namespace adcontrols {
    class Chromatogram;
}

namespace adnetcdf {

    namespace netcdf {
        class ncfile;
    }

    namespace nc = adnetcdf::netcdf;

    class AndiChromatogram {
        AndiChromatogram( const AndiChromatogram& ) = delete;
        AndiChromatogram& operator = ( const AndiChromatogram& ) = delete;
    public:
        ~AndiChromatogram();
        AndiChromatogram();
        std::shared_ptr< adcontrols::Chromatogram > import( const nc::ncfile& file ) const;

        std::optional< std::string > dataset_completeness() const; // = "C1+C2" ;
		std::optional< std::string > aia_template_revision() const; //  = "1.0.1" ;
		std::optional< std::string > netcdf_revision() const; // = "VERSION of Aug 10 2022 15:28:50 $" ;
        std::optional< std::string > languages() const; // = "English" ;
        std::optional< std::string > administrative_comments() const; // = "" ;
        std::optional< std::string > dataset_origin() const; // = "Shimadzu Corporation" ;
        std::optional< std::string > dataset_owner() const; //  = "" ;
        std::optional< std::string > dataset_date_time_stamp() const; // = "20230831150758+0900" ;
        std::optional< std::string > injection_date_time_stamp() const; //   = "20230831150758+0900" ;
        std::optional< std::string > experiment_title() const; // = "" ;
        std::optional< std::string > operator_name() const; // = "System Administrator" ;
        std::optional< std::string > separation_experiment_type() const; // = "" ;
        std::optional< std::string > company_method_name() const; // = "" ;
        std::optional< std::string > company_method_id() const; // = "" ;
        std::optional< std::string > pre_experiment_program_name() const; // = "" ;
        std::optional< std::string > post_experiment_program_name() const; // = "" ;
        std::optional< std::string > source_file_reference() const; // = "D:\\????\\COR40_1.lcd" ;
        std::optional< std::string > sample_id_comments() const; // = "" ;
        std::optional< std::string > sample_id() const; // = "" ;
        std::optional< std::string > sample_name() const; // = "CORx40" ;
        std::optional< std::string > sample_type() const; // = "Unknown" ;
        std::optional< std::string > sample_injection_volume() const; // = 1.f ;
        std::optional< std::string > sample_amount() const; // = 1.f ;
        std::optional< std::string > detection_method_table_name() const; // = "" ;
        std::optional< std::string > detection_method_comments() const; // = "" ;
        std::optional< std::string > detection_method_name() const; // = "D:\\????\\COR.lcm" ;
        std::optional< std::string > detector_name() const; // = "???o??A" ;
        std::optional< std::string > detector_unit() const; // = "Volts" ;
        std::optional< std::string > raw_data_table_name() const; // = "D:\\????\\COR40_1.lcd" ;
        std::optional< std::string > retention_unit() const; // = "Seconds" ;
        std::optional< std::string > peak_processing_results_table_name() const; // = "" ;
        std::optional< std::string > peak_processing_results_comments() const; // = "" ;
        std::optional< std::string > peak_processing_method_name() const; // = "D:\\????\\COR.lcm" ;
        std::optional< std::string > peak_processing_date_time_stamp() const; // = "" ;

    private:
        class impl;
        std::unique_ptr< impl > impl_;
    };

}
