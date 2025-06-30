/**************************************************************************
** Copyright (C) 2010-2025 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2025 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "datainterpreter.hpp"
#include "mzmlreader.hpp"
#include "mzmlspectrum.hpp"
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/waveform.hpp>
#include <adportable/bzip2.hpp>
#include <adportable/debug.hpp>
#include <adportable/serializer.hpp>

namespace mzml {

    struct translator {

        template< typename waveform_type >
        adcontrols::translate_state operator()( waveform_type& wform
                                                , const char * data, size_t dsize
                                                , const char * meta, size_t msize ) {

            // if ( meta && msize )
            //     wform.deserialize( meta, msize );

            if ( data && dsize ) {
                if ( adportable::bzip2::is_a( data, dsize ) ) {
                    std::string ar;
                    adportable::bzip2::decompress( ar, data, dsize );
                    // wform = *mzMLSpectrum::deserialize( ar.data(), ar.size() );
                } else {
                    //wform = *mzMLReader< mzml::dataTypeSpectrum >::deserialize( data, dsize );
                }
                return adcontrols::translate_complete;
            }
            return adcontrols::translate_error;
        }
    };


    // template<> adcontrols::translate_state
    // DataInterpreter<mzml::mzMLSpectrum>::translate( spectrum_t& variant, const int8_t * data, size_t dsize, const int8_t * meta, size_t msize ) {
    //     variant = mzMLSpectrum::deserialize( reinterpret_cast< const char *>(data), dsize );
    //     return adcontrols::translate_complete;
    // }


}
