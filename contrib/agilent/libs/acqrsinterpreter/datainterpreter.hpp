/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <acqrscontrols/u5303a/waveform.hpp>
#include <acqrscontrols/ap240/waveform.hpp>
#include <adcontrols/datainterpreter.hpp>
#include <adcontrols/timedigitalhistogram.hpp>
#include <boost/variant.hpp>
#include <boost/blank.hpp>

namespace adcontrols {
    class MassSpectrum;
    class TraceAccessor;
    template< typename T > class waveform;
}

namespace acqrsinterpreter {

    typedef boost::variant< std::shared_ptr< acqrscontrols::u5303a::threshold_result >
                            , std::shared_ptr< acqrscontrols::ap240::threshold_result >
                            , std::shared_ptr< acqrscontrols::u5303a::waveform >
                            , std::shared_ptr< acqrscontrols::ap240::waveform >
                            , std::shared_ptr< adcontrols::TimeDigitalHistogram > // don't move this to first item in this variant (see coadd_spectrum)
                            > waveform_types;

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
                   , const wchar_t * traceId ) const override;
        
        adcontrols::translate_state
        translate( adcontrols::TraceAccessor&
                   , const char * data, size_t dsize
                   , const char * meta, size_t msize, unsigned long events ) const override;

        bool compile_header( adcontrols::MassSpectrum&, std::ifstream& ) const override { return false; }

        bool make_device_text( std::vector< std::pair< std::string, std::string > >&
                               , const adcontrols::MSProperty& ) const override { return false; }

        virtual adcontrols::translate_state
        translate( waveform_types&, const int8_t * data, size_t dsize, const int8_t * meta, size_t msize );

        // workaround
        virtual void setWorkaroundProtocols( const std::vector< std::pair< int, double > >& p ) { op_ = p; }

    protected:
        std::vector< std::pair< int, double > > op_;
        
    private:
        void * _narrow_workaround( const char * typname ) override {
            if ( std::strcmp( typname, typeid( *this ).name() ) == 0 )
                return reinterpret_cast< void * >( this );
            else if ( std::strcmp( typname, typeid( acqrsinterpreter::DataInterpreter ).name() ) == 0 )
                return reinterpret_cast<void *>( static_cast<acqrsinterpreter::DataInterpreter *>(this) );
            return nullptr;
        }
        
    };

}
