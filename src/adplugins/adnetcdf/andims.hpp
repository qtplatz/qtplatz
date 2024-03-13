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

#pragma once

#include <adcontrols/lcmsdataset.hpp>
#include <map>
#include <optional>
#include <string>
#include <vector>
#include <memory>
#include <boost/json/fwd.hpp>

namespace adcontrols {
    class Chromatogram;
}

namespace adnetcdf {

    namespace netcdf {
        class ncfile;
    }

    namespace nc = adnetcdf::netcdf;

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

    class AndiMS : public adcontrols::LCMSDataset
                 , public std::enable_shared_from_this< AndiMS > {
        AndiMS( const AndiMS& ) = delete;
        AndiMS& operator = ( const AndiMS& ) = delete;
    public:
        ~AndiMS();
        AndiMS();
        std::vector< std::shared_ptr< adcontrols::Chromatogram > > import( const nc::ncfile& file ) const;

        std::optional< std::string > find_global_attribute( const std::string& ) const;
        const boost::json::object& json() const;
        bool has_spectra() const;
        const std::vector< data_tuple >& data() const;
        const std::map< int, std::pair< double, std::vector< int32_t > > >& transformed() const;

        // LCMSdataset
        size_t getFunctionCount() const override;
        size_t getSpectrumCount( int fcn = 0 ) const override;
        size_t getChromatogramCount() const override;
        bool getTIC( int fcn, adcontrols::Chromatogram& ) const override;
        bool getSpectrum( int fcn, size_t pos, adcontrols::MassSpectrum&, uint32_t objid ) const override;
		size_t posFromTime( double ) const override;
		double timeFromPos( size_t ) const override;
		bool getChromatograms( const std::vector< std::tuple<int, double, double> >&
                               , std::vector< adcontrols::Chromatogram >&
                               , std::function< bool (long curr, long total ) > progress
                               , int /* begPos */
                               , int /* endPos */ ) const override;
        // AcquiredDataset
        int dataformat_version() const override { return 3; }

        // v3 data support
        size_t dataReaderCount() const override;
        const adcontrols::DataReader * dataReader( size_t idx ) const override;
        const adcontrols::DataReader * dataReader( const boost::uuids::uuid& ) const override;
        std::vector < std::shared_ptr< adcontrols::DataReader > > dataReaders( bool allPossible = false ) const override;

    private:
        class impl;
        std::unique_ptr< impl > impl_;
    };

}
