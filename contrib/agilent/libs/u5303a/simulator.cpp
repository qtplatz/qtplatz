/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "simulator.hpp"
#include "digitizer.hpp"
#include "waveform_generator.hpp"
#include <workaround/boost/asio.hpp>
#include <thread>

using namespace u5303a;

simulator::simulator()
{
    const double total = 60000;
    ions_.push_back( std::make_pair( 18.0105646, 1000.0 ) ); // H2O
    ions_.push_back( std::make_pair( 28.006148,  0.7809 * total ) ); // N2
    ions_.push_back( std::make_pair( 31.9898292, 0.2095 * total ) ); // O2
    ions_.push_back( std::make_pair( 39.9623831, 0.0093 * total ) ); // Ar
}

simulator::~simulator()
{
}

bool
simulator::acquire( boost::asio::io_service& io_service )
{
    hasWaveform_ = false;

    if ( ! acqTriggered_ ) {

        io_service.post( [&]() {
                auto generator = std::make_shared< waveform_generator >( 1.0e-9, 0.0, 65535 );
                generator->addIons( ions_ );
                generator->onTriggered();
                std::this_thread::sleep_for( std::chrono::milliseconds( 1000 ) ); // simulate triggers
                post( generator.get() );
                hasWaveform_ = true;
                std::unique_lock< std::mutex > lock( queue_ );
                cond_.notify_one();
            } );

        acqTriggered_ = true;
        return true;        
    }
    return false;
}

bool
simulator::waitForEndOfAcquisition()
{
    std::unique_lock< std::mutex > lock( queue_ );
    if ( cond_.wait_for( lock, std::chrono::milliseconds( 3000 ), [=](){ return hasWaveform_ == true; } ) ) {
        acqTriggered_ = false;
        return true;
    } else {
        acqTriggered_ = false;
        return false;
    }
}

bool
simulator::readData( waveform& data )
{
    std::shared_ptr< waveform_generator > ptr;

    do {
        std::lock_guard< std::mutex > lock( mutex_ );
        if ( !waveforms_.empty() ) {
            ptr = waveforms_.front();
            waveforms_.erase( waveforms_.begin() );
        }
    } while(0);

    if ( ptr ) {
        data.d_ = ptr->waveform();
        data.meta.initialXTimeSeconds = ptr->timestamp();
        data.serialnumber = ptr->serialNumber();
        data.wellKnownEvents = 0;
        data.meta.actualElements = data.d_.size();
        data.meta.firstValidPoint = 0;
        return true;
    }
    return false;
}

void
simulator::post( waveform_generator * generator )
{
    auto ptr = generator->shared_from_this();
    std::lock_guard< std::mutex > lock( mutex_ );
    waveforms_.push_back( ptr );
}
