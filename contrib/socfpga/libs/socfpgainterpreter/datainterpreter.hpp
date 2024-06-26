/**************************************************************************
** Copyright (C) 2010-2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2019 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include <cstdint>
#include <adcontrols/datainterpreter.hpp>
#include <adcontrols/timedigitalhistogram.hpp>
#include <boost/variant.hpp>
#include <boost/blank.hpp>

namespace adcontrols {
    class MassSpectrum;
    class TraceAccessor;
    template< typename T > class waveform;
}

namespace socfpga {
    namespace dgmod {
        struct advalue;
    }
}

namespace socfpgainterpreter {

    class DataInterpreter : public adcontrols::DataInterpreter {
    public:
        virtual ~DataInterpreter();
        DataInterpreter();

        // deprecated
         adcontrols::translate_state
         translate( adcontrols::MassSpectrum&
                    , const char * data, size_t dsize
                    , const char * meta, size_t msize
                    , const adcontrols::MassSpectrometer&
                    , size_t idData
                    , const wchar_t * traceId ) const override { return adcontrols::translate_error; }
        adcontrols::translate_state
        translate( adcontrols::TraceAccessor&
                   , const char * data, size_t dsize
                   , const char * meta, size_t msize, unsigned long events ) const override;

        bool compile_header( adcontrols::MassSpectrum&, std::ifstream& ) const override { return false; }

        bool make_device_text( std::vector< std::pair< std::string, std::string > >&
                               , const adcontrols::MSProperty& ) const override { return false; }

        adcontrols::translate_state translate( std::vector< socfpga::dgmod::advalue >&, const int8_t* data, size_t dsize ) const;

        // workaround
        virtual void setWorkaroundProtocols( const std::vector< std::pair< int, double > >& p ) { op_ = p; }

    protected:
        std::vector< std::pair< int, double > > op_;

    private:
        void * _narrow_workaround( const char * typname ) override {
            if ( std::strcmp( typname, typeid( *this ).name() ) == 0 )
                return reinterpret_cast< void * >( this );
            else if ( std::strcmp( typname, typeid( socfpgainterpreter::DataInterpreter ).name() ) == 0 )
                return reinterpret_cast<void *>( static_cast<socfpgainterpreter::DataInterpreter *>(this) );
            return nullptr;
        }

    };

}
