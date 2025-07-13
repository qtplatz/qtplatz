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

#pragma once

#include "mzmlwalker.hpp"
#include <scan_protocol.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/datainterpreter.hpp>
#include <adfs/adfs.hpp>
#include <adutils/acquiredconf_v3.hpp>
#include <boost/uuid/uuid.hpp>
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
    class DataReader;
}

namespace mzml {

    class mzMLSpectrum;

    namespace local { class data_reader; }

    class mzML : public adcontrols::LCMSDataset
               , public std::enable_shared_from_this< mzML > {
        mzML( const mzML& ) = delete;
        mzML& operator = ( const mzML& ) = delete;
    public:
        ~mzML();
        mzML();
        const std::map< int, std::shared_ptr< local::data_reader > > dataReaderMap() const;

        // AcquiredDataset
        int dataformat_version() const override;

        // LCMSDataset();
        size_t getFunctionCount() const override;
        size_t getSpectrumCount( int fcn ) const override;
        size_t getChromatogramCount() const override;
        bool getTIC( int fcn, adcontrols::Chromatogram& ) const override;
        bool getSpectrum( int fcn, size_t npos, adcontrols::MassSpectrum&, uint32_t objid = 0 ) const override;
        size_t posFromTime( double x ) const override;
        double timeFromPos( size_t ) const override;
        bool index( size_t /*pos*/, int& /*idx*/, int& /*fcn*/, int& /*rep*/, double * t = 0 ) const override;
        size_t find_scan( int idx, int /* fcn */ ) const override;
        int /* idx */ make_index( size_t pos, int& fcn ) const override;
        bool getChromatograms( const std::vector< std::tuple<int, double, double> >&
                               , std::vector< adcontrols::Chromatogram >&
                               , std::function< bool (long curr, long total ) > progress
                               , int begPos = 0
                               , int endPos = (-1) ) const override;

        // --------------- v2 ???
        bool getCalibration( int, adcontrols::MSCalibrateResult&
                             , adcontrols::MassSpectrum& ) const override { return false; }
        bool hasProcessedSpectrum( int /* fcn */, int /* idx */) const override { return false; }
        uint32_t findObjId( const std::wstring& /* traceId */) const override { return 0; }
        bool getRaw( uint64_t /*objid*/, uint64_t /*npos*/
                     , uint64_t& /*fcn*/
                     , std::vector< char >& /*data*/
                     , std::vector< char >& /*meta*/ ) const override { return 0; }
        adfs::sqlite * db() const  override { return 0; }
        bool mslocker( adcontrols::lockmass::mslock&, uint32_t = 0 ) const override { return 0; }

        // v3 data support
        size_t dataReaderCount() const override;
        const adcontrols::DataReader * dataReader( size_t idx ) const override;
        const adcontrols::DataReader * dataReader( const boost::uuids::uuid& ) const override;
        std::vector < std::shared_ptr< adcontrols::DataReader > > dataReaders( bool allPossible = false ) const override;
        adcontrols::MSFractuation * msFractuation() const override;

        // mzML
        bool open( const std::filesystem::path& path );
        std::vector< std::shared_ptr< adcontrols::Chromatogram > > import_chromatograms() const;
        const std::vector< std::pair< mzml::scan_id, std::shared_ptr< mzMLSpectrum > > >& scan_indices() const;

        int get_protocol_index( const mzml::scan_protocol_key_t& key ) const;
        std::optional<mzml::scan_protocol_key_t> find_key_by_index(int index) const;

        std::pair< mzml::scan_id, std::shared_ptr< const mzml::mzMLSpectrum > >
        find_spectrum( int fcn, size_t pos, size_t rowid ) const;

        std::optional< std::pair< mzml::scan_id, std::shared_ptr< const mzml::mzMLSpectrum > > >
        find_first_spectrum( int fcn, double tR ) const;

        // export_to_adfs interface
        const pugi::xml_document& xml_document() const;
        std::optional< fileDescription > get_fileDescription() const;
        std::optional< softwareList > get_softwareList() const;
        std::optional< instrumentConfigurationList > get_instrumentConfigurationList() const;
        std::optional< dataProcessingList > get_dataProcessingList() const;

    private:
        class impl;
        std::unique_ptr< impl > impl_;
    };

}
