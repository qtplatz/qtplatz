// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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
#include <memory>

namespace adcontrols {
    class Chromatogram;
    class MassSpectrum;
    class ProcessedDataset;
	class TraceAccessor;
}

namespace addatafile {

    class rawdata : public adcontrols::LCMSDataset {
        rawdata( const rawdata& ); // noncopyable
    public:
        ~rawdata();
        rawdata( adfs::filesystem& );

        struct AcquiredConf {
            uint64_t objid;
            uint64_t pobjid;
            uint64_t trace_method;  // SignalObserver::eTRACE_METHOD
            uint64_t spectrometer;  // SignalObserver::eSPECTROMETER
            std::wstring dataInterpreterClsid;
            std::wstring trace_id;
            std::wstring trace_display_name;
            std::wstring axis_x_label;
            std::wstring axis_y_label;
            uint64_t axis_x_decimals;
            uint64_t axis_y_decimals;

            AcquiredConf();
            AcquiredConf( const AcquiredConf& );
        };

        // LCMSDataset
        size_t getFunctionCount() const override;
        size_t getSpectrumCount( int fcn = 0 ) const override;
        size_t getChromatogramCount() const override;
        bool getTIC( int fcn, adcontrols::Chromatogram& ) const override;
        bool getSpectrum( int fcn, int idx, adcontrols::MassSpectrum& ) const override;
		size_t posFromTime( double ) const override;
		bool getChromatograms( int fcn
			                         , const std::vector< std::pair<double, double> >&
			                         , std::vector< adcontrols::Chromatogram >&
									 , std::function< bool (long curr, long total ) > progress
									 , int begPos = 0
									 , int endPos = (-1) ) const override;
		bool getCalibration( int fcn, adcontrols::MSCalibrateResult&, adcontrols::MassSpectrum& ) const override;

        bool loadAcquiredConf();

    private:
        bool fetchTraces( int64_t objid, const std::wstring& clsid, adcontrols::TraceAccessor& );
        adcontrols::translate_state fetchSpectrum( int64_t objid, const std::wstring& clsid, uint64_t npos, adcontrols::MassSpectrum& ) const;
        bool readCalibration( uint32_t objid, std::wstring& dataClass, std::vector< char >& device, uint64_t& rev ) const;

        adfs::filesystem& dbf_;
        std::vector< AcquiredConf > conf_;
        std::vector< std::shared_ptr< adcontrols::Chromatogram > > tic_;
        uint64_t npos0_;
        bool configLoaded_;
    };

}

