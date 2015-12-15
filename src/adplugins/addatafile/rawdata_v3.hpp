// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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
#include <boost/variant.hpp>
#include <memory>
#include <map>
#include <cstdint>

namespace adcontrols {
    class Chromatogram;
    class MassSpectrum;
    class MassSpectrometer;
    class MSCalibrateResult;
    class ProcessedDataset;
	class TraceAccessor;
    class lockmass;
    class DataReader;
}

namespace addatafile {

    namespace v2 { typedef adutils::AcquiredConf::data conf_type; typedef std::vector< conf_type > conf_vector_type; }
    namespace v3 { typedef adutils::v3::AcquiredConf::data conf_type; typedef std::vector< conf_type > conf_vector_type; }

    namespace v3 {

        class rawdata : public adcontrols::LCMSDataset {

            rawdata( const rawdata& ) = delete;
            rawdata& operator = ( const rawdata& ) = delete;

        public:
            ~rawdata();
            rawdata( adfs::filesystem&, adcontrols::datafile& );

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

            bool loadAcquiredConf();
            void loadCalibrations();

            bool applyCalibration( const std::wstring& dataInterpreterClsid, const adcontrols::MSCalibrateResult& );

            const std::vector< std::wstring > undefined_spectrometers() const { return undefined_spectrometers_; }

            adfs::sqlite* db() override;
        
            bool mslocker( adcontrols::lockmass&, uint32_t objid ) const override;
        
        private:
            bool fetchTraces( int64_t objid, const adcontrols::DataInterpreter&, adcontrols::TraceAccessor& );

            adcontrols::translate_state fetchSpectrum( int64_t objid, const std::wstring& clsid, uint64_t npos
                                                       , adcontrols::MassSpectrum&, const std::wstring& traceId ) const;

            adfs::filesystem& dbf_;
            adcontrols::datafile& parent_;

            std::vector< adutils::v3::AcquiredConf::data > conf_;
        
            std::vector< std::shared_ptr< adcontrols::Chromatogram > > tic_;
            std::map< uint64_t, std::shared_ptr< adcontrols::MassSpectrometer > > spectrometers_; // objid,spectrometer
            std::map< uint64_t, std::shared_ptr< adcontrols::MSCalibrateResult > > calibResults_;
            uint64_t npos0_;
            bool configLoaded_;

            std::shared_ptr< adcontrols::MassSpectrometer> getSpectrometer( uint64_t objid, const std::wstring& ) const;
            std::shared_ptr< adcontrols::MassSpectrometer> getSpectrometer( uint64_t objid, const std::wstring& );
            std::vector< std::tuple< size_t, int, int> > fcnVec_; // <pos,fcn,rep,seconds>
            std::vector< std::pair< size_t, int > > fcnIdx_;
            std::vector< std::pair< double, int > > times_;
            std::vector< std::wstring > undefined_spectrometers_;
            std::vector< std::shared_ptr< adcontrols::DataReader > > readers_;
        };


    }
}

