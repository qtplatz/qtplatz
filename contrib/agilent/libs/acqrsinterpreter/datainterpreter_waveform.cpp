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

#include "datainterpreter_waveform.hpp"
#include <acqrscontrols/u5303a/waveform.hpp>
#include <acqrscontrols/ap240/waveform.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/scanlaw.hpp>
#include <adcontrols/waveform.hpp>
#include <adportable/serializer.hpp>
#include <adportable/bzip2.hpp>

namespace acqrsinterpreter {
    namespace waveform {

        struct translator {
            
            template< typename waveform_type >
            adcontrols::translate_state operator()( waveform_type& wform
                                                    , const char * data, size_t dsize
                                                    , const char * meta, size_t msize ) {

                if ( meta && msize )
                    wform.deserialize_xmeta( meta, msize );

                if ( data && dsize ) {

                    
                    if ( adportable::bzip2::is_a( data, dsize ) ) {
                        
                        std::string ar;
                        adportable::bzip2::decompress( ar, data, dsize );
                        wform.deserialize_xdata( ar.data(), ar.size() );
                        
                    } else {

                        wform.deserialize_xdata( data, dsize );                        
                        
                    }
                    return adcontrols::translate_complete;
                }
                
                return adcontrols::translate_error;
            }
        };


        ////////////////// private api ////////////////////
        // static
        template<> adcontrols::translate_state
        DataInterpreter<acqrscontrols::u5303a::waveform>::translate( acqrscontrols::u5303a::waveform& wform
                                                                     , const int8_t * data, size_t dsize, const int8_t * meta, size_t msize )
        {
            return translator()( wform, reinterpret_cast< const char *>(data), dsize, reinterpret_cast< const char *>(meta), msize );
        }

        template<> adcontrols::translate_state
        DataInterpreter<acqrscontrols::ap240::waveform>::translate( acqrscontrols::ap240::waveform& wform
                                                                    , const int8_t * data, size_t dsize, const int8_t * meta, size_t msize )
        {
            return translator()( wform, reinterpret_cast< const char *>(data), dsize, reinterpret_cast< const char *>(meta), msize );
        }        
        
        
        /////////////// public api //////////////////
        // U5303A -> MassSpectrum
        template<> adcontrols::translate_state
        DataInterpreter<acqrscontrols::u5303a::waveform>::translate( adcontrols::MassSpectrum& ms
                                                                     , const char * data, size_t dsize
                                                                     , const char * meta, size_t msize
                                                                     , const adcontrols::MassSpectrometer& spectrometer
                                                                     , size_t idData, const wchar_t * traceId ) const
        {
            auto wform = std::make_unique< acqrscontrols::u5303a::waveform >();
            
            if ( translator()( *wform, data, dsize, meta, msize ) == adcontrols::translate_complete ) {
                
                if ( auto scanlaw = spectrometer.scanLaw() )
                    acqrscontrols::u5303a::waveform::translate( ms, *wform, [&]( double t, int m ){ return scanlaw->getMass( t, m ); } );
                else
                    acqrscontrols::u5303a::waveform::translate( ms, *wform );
            }
            return adcontrols::translate_error;
        }
        
        // AP240 -> MassSpectrum
        template<> adcontrols::translate_state
        DataInterpreter<acqrscontrols::ap240::waveform>::translate( adcontrols::MassSpectrum& ms
                                                                    , const char * data, size_t dsize
                                                                    , const char * meta, size_t msize
                                                                    , const adcontrols::MassSpectrometer& spectrometer
                                                                    , size_t idData, const wchar_t * traceId ) const
        {
            auto wform = std::make_unique< acqrscontrols::ap240::waveform >();
            
            if ( translator()( *wform, data, dsize, meta, msize ) == adcontrols::translate_complete ) {
                if ( auto scanlaw = spectrometer.scanLaw() )
                    acqrscontrols::ap240::waveform::translate( ms, *wform, [&]( double t, int m ){ return scanlaw->getMass( t, m ); } );
                else
                    acqrscontrols::ap240::waveform::translate( ms, *wform );
                return adcontrols::translate_complete;
            }
            return adcontrols::translate_error;            
        }
        /////////////////////
        
    }
}

