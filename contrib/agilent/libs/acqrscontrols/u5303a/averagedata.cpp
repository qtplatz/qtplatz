/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "averagedata.hpp"
#include "method.hpp"
#include "metadata.hpp"
#include <adportable/debug.hpp>

using namespace acqrscontrols::u5303a;

AverageData::AverageData() : protocolIndex_( 0 )
                           , protocolCount_( 1 )
                           , serialnumber_( 0 )
                           , wellKnownEvents_( 0 )
                           , timeSinceEpoch_( 0 )
                           , timeSinceInject_( 0 )
{
}

AverageData::AverageData( const AverageData& t ) : protocolIndex_( t.protocolIndex_ )
                                                 , protocolCount_( t.protocolCount_ )
                                                 , meta_( t.meta_ )
                                                 , method_( t.method_ )
                                                 , serialnumber_( t.serialnumber_ )
                                                 , wellKnownEvents_( t.wellKnownEvents_ )
                                                 , timeSinceEpoch_( t.timeSinceEpoch_ )
                                                 , timeSinceInject_( t.timeSinceInject_ )
                                                 , ident_( t.ident_ )
                                                 , waveform_register_( t.waveform_register_ )
                                                 , histogram_register_( t.histogram_register_ )
{
}

void
AverageData::reset()
{
    waveform_register_.reset();
    histogram_register_.reset();
}

size_t
AverageData::average_waveform( const acqrscontrols::u5303a::waveform& waveform )
{

    typedef adportable::waveform_wrapper< int16_t, acqrscontrols::u5303a::waveform > u16wrap;
    typedef adportable::waveform_wrapper< int32_t, acqrscontrols::u5303a::waveform > u32wrap;

    ///////////
    // if ( waveform.dataType() == 2 ) {
    //     auto pair = std::minmax_element( waveform.data< int16_t >(), waveform.data< int16_t >() + waveform.size() );
    //     ADDEBUG() << "average_waveform(" << *(pair.first) << ", " << *(pair.second) << ")";
    // }
    /////////
    
    if ( ! waveform_register_ ) {

        protocolIndex_ = waveform.method_.protocolIndex();
        protocolCount_ = waveform.method_.protocols().size();
        if ( protocolIndex_ >= protocolCount_ ) {
            ADDEBUG() << "\taverage_waveform return 0 -- invalid protocol index/count "
                      << protocolIndex_ << "/" << protocolCount_;
            return 0;
        }
        
        if ( waveform.dataType() == 2 )
            waveform_register_ = std::make_shared< averager_type >( u16wrap( waveform ) );
        else
            waveform_register_ = std::make_shared< averager_type >( u32wrap( waveform ) );
        
        meta_            = waveform.meta_;
        method_          = waveform.method_;
        wellKnownEvents_ = waveform.wellKnownEvents_;
        serialnumber_    = waveform.serialnumber_;
        timeSinceEpoch_  = waveform.timeSinceEpoch_;
        timeSinceInject_ = waveform.timeSinceInject_;
        ident_           = waveform.ident_ptr();
                    
    } else {

        if ( ( protocolIndex_ != waveform.method_.protocolIndex() ) ||
             ( protocolCount_ != waveform.method_.protocols().size() ) ) {

            waveform_register_.reset(); // clear intermediate data; startover.
            
            return 0;
        }
             
        wellKnownEvents_ |= waveform.wellKnownEvents_;
        
        try {
            if ( waveform.dataType() == 2 )
                ( *waveform_register_ ) += u16wrap( waveform );
            else
                ( *waveform_register_ ) += u32wrap( waveform );

        } catch ( std::out_of_range& ) {
            reset();
            return average_waveform( waveform );
        }
    }

    meta_.actualAverages = waveform_register_->actualAverages();

    return waveform_register_->actualAverages();
}


