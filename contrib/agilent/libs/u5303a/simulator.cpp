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
#include <adicontroller/waveform_simulator_manager.hpp>
#include <adicontroller/waveform_simulator.hpp>
#include <adportable/debug.hpp>
#include <adportable/mblock.hpp>
#include <adportable/waveform_simulator.hpp>
#include <workaround/boost/asio.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/math/distributions/normal.hpp>
#include <boost/random.hpp>
#include <thread>
#include <random>
#include <memory>

namespace u5303a {

    static std::mt19937 __gen__;
    static std::uniform_real_distribution<> __dist__( -15.0, 35.0 );        
    static auto __noise__ = []{ return __dist__( __gen__ ); };
    
    static std::chrono::high_resolution_clock::time_point __uptime__ = std::chrono::high_resolution_clock::now();
    static std::chrono::high_resolution_clock::time_point __last__;
    static uint32_t __serialNumber__;
    static const std::vector< std::pair<double, double> > peak_list = { { 4.0e-6, 0.1 }, { 5.0e-6, 0.05 }, { 6.0e-6, 0.030 } };

    class waveform_simulator : public adicontroller::waveform_simulator {
    public:

        waveform_simulator( double sampInterval = 1.0e-9
                            , double startDelay = 0
                            , uint32_t nbrSamples = 100000 & 0x0f
                            , uint32_t nbrWaveforms = 1 ) : sampInterval_( sampInterval )
                                                         , startDelay_( startDelay )
                                                         , nbrSamples_( nbrSamples )
                                                         , nbrWaveforms_( nbrWaveforms )
            {}

        void addIons( const std::vector< std::pair<double, double> >& ions ) override {}
        
        void onTriggered() override;

        const int32_t * waveform() const  override { return waveform_.data(); }

        double timestamp() const  override { return timeStamp_; }

        uint32_t serialNumber() const  override { return serialNumber_; }

        double startDelay() const  override { return startDelay_; }

        uint32_t nbrWaveforms() const  override { return nbrWaveforms_; }

        uint32_t nbrSamples() const  override { return nbrSamples_; }

        double sampInterval() const  override { return sampInterval_; }

        static std::shared_ptr< adicontroller::waveform_simulator >  create( double sampInterval
                                                                           , double startDelay
                                                                           , uint32_t nbrSamples
                                                                           , uint32_t nbrWaveforms ) {
            return std::make_shared< waveform_simulator >( sampInterval, startDelay, nbrSamples, nbrWaveforms );
        }
        
        std::vector< int32_t > waveform_;
        double startDelay_;
        double sampInterval_;
        double timeStamp_;
        uint32_t serialNumber_;
        uint32_t nbrSamples_;
        uint32_t nbrWaveforms_;
    };

}

//waveform_simulator_generator_t __waveform_simulator_generator;

using namespace u5303a;

simulator::simulator() : hasWaveform_( false )
                       , sampInterval_( 1.0e-9 )
                       , startDelay_( 0.0 )
                       , nbrSamples_( 10000 & ~0x0f )
                       , nbrWaveforms_( 496 )
                       , exitDelay_( 0.0 )
                       , method_( std::make_shared< acqrscontrols::u5303a::method >() )
{
    acqTriggered_.clear();

    if ( ! adicontroller::waveform_simulator_manager::instance().waveform_simulator( 0, 0, 0, 0 ) ) {

        // No external simulator found, install local simulator
        adicontroller::waveform_simulator_manager::instance().install_factory( [](double _1, double _2, uint32_t _3, uint32_t _4){
                return std::make_shared< waveform_simulator >(_1, _2, _3, _4);
            });

    }

    // for InfiTOF simulator compatibility
    const double total = 60000;
    ions_.push_back( std::make_pair( 18.0105646, 1000.0 ) ); // H2O
    ions_.push_back( std::make_pair( 28.006148,  0.7809 * total ) ); // N2
    ions_.push_back( std::make_pair( 31.9898292, 0.2095 * total ) ); // O2
    ions_.push_back( std::make_pair( 39.9623831, 0.0093 * total ) ); // Ar
}

simulator::~simulator()
{
}

simulator *
simulator::instance()
{
    static simulator __simulator__;
    return &__simulator__;
}

void
simulator::protocol_handler( double delay, double width )
{
    exitDelay_ = delay;
    (void)width;
}

bool
simulator::acquire()
{
    hasWaveform_ = false;

    if ( ! acqTriggered_.test_and_set() ) {

		if ( auto generator = adicontroller::waveform_simulator_manager::instance().waveform_simulator( sampInterval_, startDelay_, nbrSamples_, nbrWaveforms_ ) ) {
                
			generator->addIons( ions_ );
			generator->onTriggered();
			post( generator );
			
			hasWaveform_ = true;
			std::unique_lock< std::mutex > lock( queue_ );
			cond_.notify_one();
		}
	}
	return true;
}

bool
simulator::waitForEndOfAcquisition()
{
    //std::this_thread::sleep_for( std::chrono::milliseconds( nbrWaveforms_ ) ); // simulate triggers
    std::unique_lock< std::mutex > lock( queue_ );
    if ( cond_.wait_for( lock, std::chrono::milliseconds( 3000 ), [=](){ return hasWaveform_ == true; } ) ) {
        acqTriggered_.clear();
        return true;
    } else {
        acqTriggered_.clear();
        return false;
    }
}

bool
simulator::readData( acqrscontrols::u5303a::waveform& data )
{
    std::shared_ptr< adicontroller::waveform_simulator > ptr;

    do {
        std::lock_guard< std::mutex > lock( mutex_ );
        if ( !waveforms_.empty() ) {
            ptr = waveforms_.front();
            waveforms_.erase( waveforms_.begin() );
        }
    } while(0);
    
    if ( ptr ) {
		auto mblk = std::make_shared< adportable::mblock<int32_t> >( ptr->nbrSamples() );

        auto dp = mblk->data();
        std::copy( ptr->waveform(), ptr->waveform() + ptr->nbrSamples(), dp );
        data.method_ = *method_;
        data.method_.method_.digitizer_delay_to_first_sample = startDelay_;
        data.method_.method_.nbr_of_averages = int32_t( nbrWaveforms_ );
        data.method_.method_.digitizer_nbr_of_s_to_acquire = int32_t( nbrSamples_ );

        data.meta_.initialXTimeSeconds = ptr->timestamp();
        data.wellKnownEvents_ = 0;
        data.meta_.actualPoints = ptr->nbrSamples();
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
simulator::post( std::shared_ptr< adicontroller::waveform_simulator >& generator )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    waveforms_.push_back( generator );
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

void
simulator::touchup( std::vector< std::shared_ptr< acqrscontrols::u5303a::waveform > >& vec )
{
    static size_t counter;
    
    if ( ! vec.empty() )  {

        auto& w = *vec[ 0 ];
                
        if ( w.meta_.dataType == 2 ) {

            std::shared_ptr< adportable::mblock< int16_t > > mblock;
            adportable::waveform_simulator( w.meta_.initialXOffset, w.meta_.actualPoints, w.meta_.xIncrement )( mblock, int( vec.size() ) );

            for ( auto& w: vec ) {
                w->setData( mblock, w->firstValidPoint_ );
                if ( w->meta_.initialXTimeSeconds == 0 )
                    w->meta_.initialXTimeSeconds = counter++;
            }

        } else {

            std::shared_ptr< adportable::mblock< int16_t > > mblock;
            adportable::waveform_simulator()( mblock, int( vec.size() ) );
            for ( auto& w: vec )
                w->setData( mblock, w->firstValidPoint_ );

        }
    }
}

///////////////////////////////

void
waveform_simulator::onTriggered()
{
	std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();    
    timeStamp_ = std::chrono::duration< double >( now - __uptime__ ).count(); // s

    double seconds = timeStamp_;
    double hf = ( std::sin( seconds / 10.0 ) + 1.20 ) / 2.20;

	static int counter;
    
	__last__ = now;

    waveform_.resize( nbrSamples_ );

    size_t idx = 0;
    for ( int32_t& d: waveform_ ) {
        
        double t = startDelay_ + sampInterval_ * idx++;

        double y = 0;
        for ( auto& peak : peak_list ) {
            boost::math::normal_distribution< double > nd( peak.first /* mean */, 5.0e-9 /* sd */);
            y += boost::math::pdf( nd, t ) * peak.second; // + __noise__();
        }
        d = int32_t( int32_t( y ) + __noise__() );
    }
}


