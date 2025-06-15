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

#include <adcontrols/datafile.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/datainterpreter.hpp>
#include <adfs/adfs.hpp>
#include <adutils/acquiredconf.hpp>
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

    class mzML : public adcontrols::LCMSDataset {
        mzML( const mzML& ) = delete;
        mzML& operator = ( const mzML& ) = delete;
    public:
        ~mzML();
        mzML();

        // LCMSDataset();
        size_t getFunctionCount() const;
        size_t getSpectrumCount( int fcn ) const;
        size_t getChromatogramCount() const;
        bool getTIC( int fcn, adcontrols::Chromatogram& ) const;
        bool getSpectrum( int fcn, size_t npos, adcontrols::MassSpectrum&, uint32_t objid = 0 ) const;
        size_t posFromTime( double x ) const;
        double timeFromPos( size_t ) const;
        bool index( size_t /*pos*/, int& /*idx*/, int& /*fcn*/, int& /*rep*/, double * t = 0 ) const;
        size_t find_scan( int idx, int /* fcn */ ) const;
        int /* idx */ make_index( size_t pos, int& fcn ) const;
        bool getChromatograms( const std::vector< std::tuple<int, double, double> >&
                               , std::vector< adcontrols::Chromatogram >&
                               , std::function< bool (long curr, long total ) > progress
                               , int begPos = 0
                               , int endPos = (-1) ) const;

        // --------------- v2 ???
        bool getCalibration( int, adcontrols::MSCalibrateResult&, adcontrols::MassSpectrum& ) const { return false; }
        bool hasProcessedSpectrum( int /* fcn */, int /* idx */) const { return false; }
        uint32_t findObjId( const std::wstring& /* traceId */) const { return 0; }
        bool getRaw( uint64_t /*objid*/, uint64_t /*npos*/
                     , uint64_t& /*fcn*/, std::vector< char >& /*data*/, std::vector< char >& /*meta*/ ) const { return 0; }
        adfs::sqlite * db() const { return 0; }
        bool mslocker( adcontrols::lockmass::mslock&, uint32_t = 0 ) const { return 0; }

        // v3 data support
        size_t dataReaderCount() const;
        const adcontrols::DataReader * dataReader( size_t idx ) const;
        const adcontrols::DataReader * dataReader( const boost::uuids::uuid& ) const;
        std::vector < std::shared_ptr< adcontrols::DataReader > > dataReaders( bool allPossible = false ) const;
        adcontrols::MSFractuation * msFractuation() const;

        // mzML
        bool open( const std::filesystem::path& path );
    private:
        class impl;
        std::unique_ptr< impl > impl_;
    };

}
