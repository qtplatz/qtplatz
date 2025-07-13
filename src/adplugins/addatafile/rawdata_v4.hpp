// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2019 MS-Cheminformatics LLC
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

#include <adcontrols/datafile.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/datainterpreter.hpp>
#include <adfs/adfs.hpp>
#include <adutils/acquiredconf.hpp>
#include <adutils/acquiredconf_v3.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/variant.hpp>
#include <memory>
#include <cstdint>

namespace adcontrols {
    class Chromatogram;
    class MassSpectrum;
    class MassSpectrometer;
    class MSCalibrateResult;
    class ProcessedDataset;
	class TraceAccessor;
    class DataReader;
}

namespace addatafile {

    namespace v2 { typedef adutils::AcquiredConf::data conf_type; typedef std::vector< conf_type > conf_vector_type; }
    namespace v3 { typedef adutils::v3::AcquiredConf::data conf_type; typedef std::vector< conf_type > conf_vector_type; }

    namespace v4 {
        // rawdata bridge for external raw data (such as imported binary from mzML file)

        class rawdata : public adcontrols::LCMSDataset {

            rawdata( const rawdata& ) = delete;
            rawdata& operator = ( const rawdata& ) = delete;

        public:
            ~rawdata();
            rawdata( std::shared_ptr< adfs::sqlite > );

            // AcquiredDataset
            int dataformat_version() const override { return 3; }

            // LCMSDataset
            size_t getFunctionCount() const override;
            size_t getSpectrumCount( int fcn = 0 ) const override;
            size_t getChromatogramCount() const override;
            bool getTIC( int fcn, adcontrols::Chromatogram& ) const override;
            bool getSpectrum( int fcn, size_t idx, adcontrols::MassSpectrum&, uint32_t objId ) const override;
            size_t posFromTime( double ) const override;
            double timeFromPos( size_t ) const override;
            bool index( size_t pos, int& idx, int& fcn, int& rep, double * t ) const override;
            size_t find_scan( int idx, int fcn ) const override;
            int make_index( size_t pos, int& fcn ) const override;

            bool getChromatograms( const std::vector< std::tuple<int, double, double> >&
                                   , std::vector< adcontrols::Chromatogram >&
                                   , std::function< bool (long curr, long total ) > progress
                                   , int begPos = 0
                                   , int endPos = (-1) ) const override;
            bool hasProcessedSpectrum( int, int ) const override;
            uint32_t findObjId( const std::wstring& traceId ) const override;
            bool getRaw( uint64_t objid, uint64_t npos, uint64_t& fcn, std::vector< char >& data, std::vector< char >& meta ) const override;

            //

            bool loadAcquiredConf();
            bool loadCalibrations();
            void loadMSFractuation();

            bool applyCalibration( const adcontrols::MSCalibrateResult& );

            const std::vector< std::string > undefined_spectrometers() const { return undefined_spectrometers_; }
            const std::vector< std::pair< std::string, boost::uuids::uuid > > undefined_data_readers() const { return undefined_data_readers_; }

            adfs::sqlite* db() const override;

            bool mslocker( adcontrols::lockmass::mslock&, uint32_t objid ) const override;

            // v3 specific
            size_t dataReaderCount() const override;
            const adcontrols::DataReader * dataReader( size_t idx ) const override;
            const adcontrols::DataReader * dataReader( const boost::uuids::uuid& ) const override;
            std::vector < std::shared_ptr< adcontrols::DataReader > > dataReaders( bool allPossible ) const override;

        private:
            bool fetchTraces( int64_t objid, const adcontrols::DataInterpreter&, adcontrols::TraceAccessor& );

            adcontrols::translate_state fetchSpectrum( int64_t objid, const std::wstring& clsid, uint64_t npos
                                                       , adcontrols::MassSpectrum&, const std::wstring& traceId ) const;

            std::shared_ptr< adcontrols::DataReader > findDataReader( int fcn, int& xfcn ) const;

            std::shared_ptr< adfs::sqlite > db_;

            std::vector< adutils::v3::AcquiredConf::data > conf_;

            int32_t fcnCount_;

            std::vector< std::shared_ptr< adcontrols::Chromatogram > > tic_;
            //std::map< uint64_t, std::shared_ptr< adcontrols::MassSpectrometer > > spectrometers_; // objid,spectrometer
            //std::map< uint64_t, std::shared_ptr< adcontrols::MSCalibrateResult > > calibResults_;
            uint64_t npos0_;
            bool configLoaded_;

            std::shared_ptr< adcontrols::MassSpectrometer> getSpectrometer( uint64_t objid, const std::wstring& ) const;
            //std::shared_ptr< adcontrols::MassSpectrometer> getSpectrometer( uint64_t objid, const std::wstring& );
            std::vector< std::tuple< size_t, int, int> > fcnVec_; // <pos,fcn,rep,seconds>
            std::vector< std::pair< size_t, int > > fcnIdx_;
            std::vector< std::pair< double, int > > times_;
            std::vector< std::string > undefined_spectrometers_;
            std::vector< std::pair< std::string, boost::uuids::uuid > > undefined_data_readers_;
            std::vector< std::pair< std::shared_ptr< adcontrols::DataReader >, int > > readers_; // <reader,fcn>
        };


    }
}
