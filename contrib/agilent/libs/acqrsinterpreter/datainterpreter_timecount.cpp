/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <acqrscontrols/ap240/threshold_result.hpp>
#include <adcontrols/waveform.hpp>
#include <adportable/debug.hpp>
#include <adcontrols/timedigitalhistogram.hpp>
#include <adportable/serializer.hpp>
#include <adportable/bzip2.hpp>

using namespace acqrsinterpreter::timecount;

namespace acqrsinterpreter {

    namespace timecount {

        // U5303A interface
        template<> adcontrols::translate_state
        DataInterpreter<acqrscontrols::u5303a::threshold_result>::translate( std::shared_ptr< acqrscontrols::u5303a::threshold_result >& native
                                                                             , const int8_t * data, size_t dsize, const int8_t * meta, size_t msize )
        {
            if ( data && dsize ) {
                
                if ( adportable::bzip2::is_a( reinterpret_cast<const char *>( data ), dsize ) ) {
                    
                    std::string ar;            
                    adportable::bzip2::decompress( ar, reinterpret_cast<const char *>( data ), dsize );
                    native->deserialize( reinterpret_cast< const int8_t *>(ar.data()), ar.size(), meta, msize );
                    
                } else {
                    
                    native->deserialize( data, dsize, meta, msize );
                    
                }
                return adcontrols::translate_complete;
            }
            
            return adcontrols::translate_error;
        }

        // AP240 interface
        template<> adcontrols::translate_state
        DataInterpreter<acqrscontrols::ap240::threshold_result>::translate( std::shared_ptr< acqrscontrols::ap240::threshold_result >& native
                                                                             , const int8_t * data, size_t dsize, const int8_t * meta, size_t msize )
        {
            if ( data && dsize ) {
                
                if ( adportable::bzip2::is_a( reinterpret_cast<const char *>( data ), dsize ) ) {
                    
                    std::string ar;            
                    adportable::bzip2::decompress( ar, reinterpret_cast<const char *>( data ), dsize );
                    native->deserialize( reinterpret_cast< const int8_t *>(ar.data()), ar.size(), meta, msize );
                    
                } else {
                    
                    native->deserialize( data, dsize, meta, msize );
                    
                }
                return adcontrols::translate_complete;
            }
            
            return adcontrols::translate_error;
        }


        
    }
}
