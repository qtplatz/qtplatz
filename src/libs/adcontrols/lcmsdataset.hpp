// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include "adcontrols_global.h"
#include "acquireddataset.hpp"
#include <functional>
#include <vector>
#include <string>
#include <tuple>
#include <cstdint>
#include <sys/types.h>
#include <memory>

namespace adfs { class sqlite; }
namespace boost { namespace uuids { struct uuid; } }

namespace adcontrols {

    namespace lockmass { class mslock; }

    class Chromatogram;
    class MassSpectrum;
	class MSCalibrateResult;
    class MSFractuation;

    // v3 dataformat only
    class DataReader;
    //

    class ADCONTROLSSHARED_EXPORT LCMSDataset : public AcquiredDataset {
    public:
        // LCMSDataset();
        virtual size_t getFunctionCount() const = 0;
        virtual size_t getSpectrumCount( int fcn ) const = 0;
        virtual size_t getChromatogramCount() const = 0;
        virtual bool getTIC( int fcn, adcontrols::Chromatogram& ) const = 0;
        virtual bool getSpectrum( int fcn, size_t npos, adcontrols::MassSpectrum&, uint32_t objid = 0 ) const = 0;
		virtual size_t posFromTime( double x ) const = 0;
        virtual double timeFromPos( size_t ) const = 0;
        virtual bool index( size_t /*pos*/, int& /*idx*/, int& /*fcn*/, int& /*rep*/, double * t = 0 ) const { return false; (void)t; }
        virtual size_t find_scan( int idx, int /* fcn */ ) const { return idx; }
        virtual int /* idx */ make_index( size_t pos, int& fcn ) const { fcn = 0; return int(pos); }
		virtual bool getChromatograms( const std::vector< std::tuple<int, double, double> >&
			                         , std::vector< adcontrols::Chromatogram >&
									 , std::function< bool (long curr, long total ) > progress
									 , int begPos = 0
									 , int endPos = (-1) ) const = 0;
		virtual bool getCalibration( int, MSCalibrateResult&, MassSpectrum& ) const { return false; }
		virtual bool hasProcessedSpectrum( int /* fcn */, int /* idx */) const { return false; } // compassXpress return true for centroid result
        virtual uint32_t findObjId( const std::wstring& /* traceId */) const { return 0; }
        virtual bool getRaw( uint64_t /*objid*/, uint64_t /*npos*/
                             , uint64_t& /*fcn*/, std::vector< char >& /*data*/, std::vector< char >& /*meta*/ ) const { return 0; }
        virtual adfs::sqlite * db() const { return 0; }
        virtual bool mslocker( lockmass::mslock&, uint32_t = 0 ) const { return 0; }

        // v3 data support
        virtual size_t dataReaderCount() const { return 0; }
        virtual const adcontrols::DataReader * dataReader( size_t idx ) const { return nullptr; }
        virtual const adcontrols::DataReader * dataReader( const boost::uuids::uuid& ) const { return nullptr; }
        virtual std::vector < std::shared_ptr< adcontrols::DataReader > > dataReaders( bool allPossible = false ) const;
        virtual adcontrols::MSFractuation * msFractuation() const;
	};

}
