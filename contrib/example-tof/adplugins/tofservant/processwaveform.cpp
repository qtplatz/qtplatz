/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include "processwaveform.hpp"
#include "constants.h"
#include <tofinterface/tofC.h>

#include "profileobserver_i.hpp"
#include "traceobserver_i.hpp"
#include <adportable/spectrum_processor.hpp>
#include <ace/Message_Block.h>

using namespace tofservant;

ProcessWaveform::~ProcessWaveform()
{
    ACE_Message_Block::release( mblk_ );
}

ProcessWaveform::ProcessWaveform( ACE_Message_Block * mblk ) : mblk_( mblk->duplicate() )
                                                             , npos_( 0 )
                                                             , wellKnownEvents_( 0 )
                                                             , data_( new TOFSignal::tofDATA )
{
    using namespace tofservant::constants;

    assert( mblk->msg_type() == MB_TOF_DATA );

    if ( mblk->msg_type() != MB_TOF_DATA )
        return;
 
    TAO_InputCDR in( mblk->cont() );
    if ( ! ( in >> *data_ ) ) {
        data_.reset();
        return;
    }
    npos_ = data_->sequenceNumber;

}

bool
ProcessWaveform::push_traces( std::vector< std::shared_ptr< traceObserver_i > >& vec )
{
    if ( vec.empty() || mblk_ == 0 )
        return false;

    if ( data_ ) {

        const TOFSignal::tofDATA& data = *data_;
        if ( data.data.length() > 0 ) {
            size_t ndata = data.data[ 0 ].values.length();

            double dbase(0), rms(0);
            if ( ndata > 0 ) {
                const TOFSignal::datum& datum = data.data[ 0 ];
                const long * waveform = reinterpret_cast< const long * >( &datum.values[0] );
                size_t nbrSamples = datum.values.length();
                double tic = adportable::spectrum_processor::tic( nbrSamples, waveform, dbase, rms ); 

                std::cout << "ProcessWaveform: push_traces: "
                          << npos_ << " nbrSamples: " << ndata << " tic: " << tic << std::endl;

                TOFSignal::SpectrumProcessedData procData;
                procData.tic = float( tic );
                procData.spectralBaselineLevel = float( dbase );
                procData.uptime = data.clockTimeStamp;
                
                TOFSignal::TraceMetadata meta;
                meta.uptime = procData.uptime;
                meta.timeSinceInject = static_cast< CORBA::ULong >( data.clockTimeStamp / 1000 ); // us -> ms
                meta.wellKnownEvents = 0;
                meta.sampInterval = 1000; // workaround
                vec[ 0 ]->push_back( npos_, procData, meta );
            }
            return true;
        }
    }
    return false;
}

bool
ProcessWaveform::push_profile( profileObserver_i * profileObserver )
{
    if ( profileObserver == 0 || mblk_ == 0 )
        return false;
    profileObserver->push_profile_data( mblk_, npos_, wellKnownEvents_ );
    return true;
}

