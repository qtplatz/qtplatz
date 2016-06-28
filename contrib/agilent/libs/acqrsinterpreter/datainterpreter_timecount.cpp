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

#include "datainterpreter_timecount.hpp"
#include <acqrscontrols/u5303a/threshold_result.hpp>
#include <acqrscontrols/u5303a/histogram.hpp>
#include <acqrscontrols/ap240/threshold_result.hpp>
#include <acqrscontrols/ap240/histogram.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/scanlaw.hpp>
#include <adcontrols/waveform.hpp>
#include <adportable/debug.hpp>
#include <adcontrols/timedigitalhistogram.hpp>
#include <adportable/serializer.hpp>
#include <adportable/bzip2.hpp>

// using namespace acqrsinterpreter::timecount;

namespace acqrsinterpreter {

    namespace timecount {

        template< typename result_type >
        struct translator1 {

            // serialized data --> acqrscontrols::<digitizer>::threshold_result 
            adcontrols::translate_state operator()( result_type& native
                                                    , const int8_t * data, size_t dsize
                                                    , const int8_t * meta, size_t msize ) {
                if ( data && dsize ) {
                    
                    if ( adportable::bzip2::is_a( reinterpret_cast<const char *>( data ), dsize ) ) {
                        
                        std::string ar;
                        adportable::bzip2::decompress( ar, reinterpret_cast<const char *>( data ), dsize );
                        native.deserialize( reinterpret_cast< const int8_t *>(ar.data()), ar.size(), meta, msize );
                        
                    } else {
                        
                        native.deserialize( data, dsize, meta, msize );
                        
                    }
                    return adcontrols::translate_complete;
                }
                
                return adcontrols::translate_error;
            }
        };
        

        template< typename result_type, typename histogram_type >
        struct translator {
            
            // serialized data --> MassSpectrum
            adcontrols::translate_state operator()( adcontrols::MassSpectrum& ms
                                                    , const char * data, size_t dsize
                                                    , const char * meta, size_t msize
                                                    , const adcontrols::MassSpectrometer& spectrometer ) {

                auto result = std::make_unique< result_type >();

                if ( translator1<result_type>()( *result
                                                 , reinterpret_cast< const int8_t * >( data ), dsize
                                                 , reinterpret_cast< const int8_t * >( meta ), msize ) ) {

                    histogram_type hgrm; // auto hgrm = std::make_unique< acqrscontrols::ap240::histogram >();
                    
                    hgrm.append( *result );

                    adcontrols::TimeDigitalHistogram histogram;
                    hgrm.move( histogram );

                    if ( auto scanlaw = spectrometer.scanLaw() )
                        adcontrols::TimeDigitalHistogram::translate( ms, histogram, [&](double t, int m){ return scanlaw->getMass(t,m); });
                    else
                        adcontrols::TimeDigitalHistogram::translate( ms, histogram );

                    return adcontrols::translate_complete;

                }
                return adcontrols::translate_error;
            }
                
        };
        
        // private interface for 
        template<> adcontrols::translate_state
        DataInterpreter<acqrscontrols::u5303a::threshold_result>::translate( acqrscontrols::u5303a::threshold_result& native
                                                                             , const int8_t * data, size_t dsize
                                                                             , const int8_t * meta, size_t msize )
        {
            return translator1<acqrscontrols::u5303a::threshold_result>()( native, data, dsize, meta, msize );
        }

        // AP240 interface
        template<> adcontrols::translate_state
        DataInterpreter<acqrscontrols::ap240::threshold_result>::translate( acqrscontrols::ap240::threshold_result& native
                                                                            , const int8_t * data, size_t dsize
                                                                            , const int8_t * meta, size_t msize )
        {
            return translator1<acqrscontrols::ap240::threshold_result>()( native, data, dsize, meta, msize );
        }
        //----------------------------------------------------
        //----------------------------------------------------

        /////////////// public api //////////////////
        // U5303A -> MassSpectrum
        template<> adcontrols::translate_state
        DataInterpreter<acqrscontrols::u5303a::threshold_result>::translate( adcontrols::MassSpectrum& ms
                                                                             , const char * data, size_t dsize
                                                                             , const char * meta, size_t msize
                                                                             , const adcontrols::MassSpectrometer& spectrometer
                                                                             , size_t idData, const wchar_t * traceId ) const
        {
            return 
                translator< acqrscontrols::u5303a::threshold_result
                            , acqrscontrols::u5303a::histogram >()( ms, data, dsize, meta, msize, spectrometer );
        }
        
        // AP240 -> MassSpectrum
        template<> adcontrols::translate_state
        DataInterpreter<acqrscontrols::ap240::threshold_result>::translate( adcontrols::MassSpectrum& ms
                                                                            , const char * data, size_t dsize
                                                                            , const char * meta, size_t msize
                                                                            , const adcontrols::MassSpectrometer& spectrometer
                                                                            , size_t idData, const wchar_t * traceId ) const
        {
            return 
                translator< acqrscontrols::ap240::threshold_result
                            , acqrscontrols::ap240::histogram >()( ms, data, dsize, meta, msize, spectrometer );
        }

        /////////////////////
        
    } // namespace timecount
}
