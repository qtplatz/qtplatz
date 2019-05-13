// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
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

#include <adcontrols/datainterpreter.hpp>
#include <vector>
#include <string>
#include <memory>

namespace multumcontrols { class ScanLaw; }
namespace adcontrols { namespace lockmass { class mslock; } class ProcessMethod; }

namespace infitofspectrometer {

    class InfiTofDataInterpreter : public adcontrols::DataInterpreter {
    public:
        InfiTofDataInterpreter(void);
        virtual ~InfiTofDataInterpreter(void);

		static double compute_mass( double time, int mode, const adcontrols::MSCalibration&, const multumcontrols::ScanLaw& );

        adcontrols::translate_state translate( adcontrols::MassSpectrum&
                                               , const char * data, size_t dsize
                                               , const char * meta, size_t msize
                                               , const adcontrols::MassSpectrometer&
                                               , size_t idData
											   , const wchar_t * traceId ) const override;
        
        adcontrols::translate_state translate( adcontrols::TraceAccessor&
                                               , const char * data, size_t dsize
                                               , const char * meta, size_t msize, unsigned long events ) const override;

        bool compile_header( adcontrols::MassSpectrum&, std::ifstream& ) const override;

        bool make_device_text( std::vector< std::pair< std::string, std::string > >&, const adcontrols::MSProperty& ) const override;

        bool has_lockmass() const override;
        bool lockmass( adcontrols::lockmass::mslock& ) const override;
        
        // local implementation
        void set_logfile( const char * logfile );
        void setProcessMethod( const adcontrols::ProcessMethod& );

    private:
        std::shared_ptr< adcontrols::lockmass::mslock > mslocker_;
        std::shared_ptr< adcontrols::ProcessMethod > pm_;
        std::string logfile_;
        double peakwidth_;
        size_t npeaksamples_;
    };

}
