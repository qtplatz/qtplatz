// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2019 MS-Cheminformatics LLC
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
#include "massspectrum.hpp"
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace adcontrols {

    struct waveform_translator {

        template< typename waveform_type >
        static bool translate( MassSpectrum& sp
                               , const waveform_type& waveform
                               , double xIncrement
                               , double trigDelay
                               , const char * dataInterpreterClsid
                               , const std::string& device_data ) {

            adcontrols::MSProperty prop = sp.getMSProperty();
            adcontrols::SamplingInfo info( xIncrement
                                           , trigDelay
                                           , int32_t( trigDelay / xIncrement )
                                           , uint32_t( waveform.size() )
                                           , 0 // waveform.meta_.actualAverages
                                           , 0 // mode );
                );
            info.setSampInterval( xIncrement );

            prop.setSamplingInfo( info );
            prop.setTDelay( trigDelay );
            prop.setTimeSinceInjection( double(waveform.elapsed_time()) / std::nano::den );
            prop.setTimeSinceEpoch( waveform.epoch_time() ); // nanoseconds
            prop.setDataInterpreterClsid( dataInterpreterClsid );
            prop.setDeviceData( device_data.data(), device_data.size() );
            prop.setTrigNumber( waveform.serialnumber() );

            std::vector< double > d( waveform.size() );
            std::copy( waveform.begin(), waveform.end(), d.begin() );

            sp.setCentroid( adcontrols::CentroidNone );
            sp.setMSProperty( prop );
            sp.setIntensityArray( std::move( d ) );

            return true;
        }
    };
}
