/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <adportable/debug.hpp>
#include <adinterface/waveform_generator.hpp>
#include <workaround/boost/asio.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <thread>

namespace u5303a {
    simulator * simulator::instance_(0);
}

waveform_generator_generator_t __waveform_generator_generator;

using namespace u5303a;

simulator::simulator() : sampInterval_( 1.0e-9 )
                       , startDelay_( 0.0 )
                       , nbrSamples_( 10000 & ~0x0f )
                       , nbrWaveforms_( 496 )
                       , exitDelay_( 0.0 )
                       , method_( std::make_shared< acqrscontrols::u5303a::method >() )
{
    try {
        boost::interprocess::managed_shared_memory shm( boost::interprocess::open_only, "waveform_simulator" );

        if ( auto mx = shm.find_or_construct< boost::interprocess::interprocess_mutex >( "waveform_simulator_mutex" )( ) ) {

            auto ptr = shm.find< waveform_generator_generator_t >( "waveform_generator_generator" );

            if ( ptr.first ) {

                boost::interprocess::scoped_lock< boost::interprocess::interprocess_mutex > lock( *mx );
                if ( __waveform_generator_generator = *ptr.first )
                    auto p = __waveform_generator_generator( 0, 0, 0, 0 );

            }
        }
    } catch ( std::exception& ex ) {
        ADDEBUG() << ex.what();
    }

    instance_ = this;

    const double total = 60000;
    ions_.push_back( std::make_pair( 18.0105646, 1000.0 ) ); // H2O
    ions_.push_back( std::make_pair( 28.006148,  0.7809 * total ) ); // N2
    ions_.push_back( std::make_pair( 31.9898292, 0.2095 * total ) ); // O2
    ions_.push_back( std::make_pair( 39.9623831, 0.0093 * total ) ); // Ar
}

simulator::~simulator()
{
    instance_ = 0;
}

simulator *
simulator::instance()
{
    return instance_;
}

void
simulator::protocol_handler( double delay, double width )
{
    exitDelay_ = delay;
    (void)width;
}

bool
simulator::acquire( boost::asio::io_service& io_service )
{
    hasWaveform_ = false;

    if ( ! acqTriggered_ ) {

        io_service.post( [&]() {
            if ( __waveform_generator_generator ) {

                if ( auto generator = __waveform_generator_generator( sampInterval_, startDelay_, nbrSamples_, nbrWaveforms_ ) ) {

                    generator->addIons( ions_ );
                    generator->onTriggered();

                    std::this_thread::sleep_for( std::chrono::milliseconds( nbrWaveforms_ ) ); // simulate triggers

                    post( generator.get() );

                    hasWaveform_ = true;
                    std::unique_lock< std::mutex > lock( queue_ );
                    cond_.notify_one();
                }
            }
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
simulator::readData( acqrscontrols::u5303a::waveform& data )
{
    std::shared_ptr< adinterface::waveform_generator > ptr;

    do {
        std::lock_guard< std::mutex > lock( mutex_ );
        if ( !waveforms_.empty() ) {
            ptr = waveforms_.front();
            waveforms_.erase( waveforms_.begin() );
        }
    } while(0);

    if ( ptr ) {
        auto dp = data.data( ptr->nbrSamples() );
        std::copy( ptr->waveform(), ptr->waveform() + ptr->nbrSamples(), dp );
        data.method_ = *method_;
        data.method_.method_.digitizer_delay_to_first_sample = startDelay_;
        data.method_.method_.nbr_of_averages = int32_t( nbrWaveforms_ );
        data.method_.method_.digitizer_nbr_of_s_to_acquire = int32_t( nbrSamples_ );

        data.meta_.initialXTimeSeconds = ptr->timestamp();
        data.serialnumber_ = ptr->serialNumber();
        data.wellKnownEvents_ = 0;
        data.meta_.actualPoints = data.data_size();
        data.meta_.xIncrement = sampInterval_;
        data.meta_.initialXOffset = startDelay_;
        data.meta_.actualAverages = int32_t( nbrWaveforms_ );
        data.meta_.scaleFactor = 1.0;
        data.meta_.scaleOffset = 0.0;

        return true;
    }
    return false;
}

void
simulator::post( adinterface::waveform_generator * generator )
{
    auto ptr = generator->shared_from_this();
    std::lock_guard< std::mutex > lock( mutex_ );
    waveforms_.push_back( ptr );
}

void
simulator::setup( const acqrscontrols::u5303a::method& m )
{
    *method_ = m;
    sampInterval_ = 1.0 / m.method_.samp_rate;
    startDelay_ = m.method_.digitizer_delay_to_first_sample;
    nbrSamples_ = m.method_.digitizer_nbr_of_s_to_acquire;
    nbrWaveforms_ = m.method_.nbr_of_averages;
}
