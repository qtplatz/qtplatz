// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#include "time_event_processor.hpp"
#include "task.hpp"
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/controlmethod/timedevent.hpp>
#include <adcontrols/controlmethod/timedevents.hpp>
#include <adportable/float.hpp>
#include <adportable/debug.hpp>

using namespace adicontroller;

time_event_processor::time_event_processor() : methodTime_( 60.0 )
                                             , deadline_timer_( task::instance()->io_service() ) {
}

time_event_processor::~time_event_processor()
{
    deadline_timer_.cancel();
}

double
time_event_processor::methodTime() const
{
    return methodTime_;
}

void
time_event_processor::setControlMethod( std::shared_ptr< const adcontrols::ControlMethod::Method > m, double methodTime )
{
    ttable_.reset();

    method_ = m;
    methodTime_ = methodTime;
    
    auto it = m->find( m->begin(), m->end(), adcontrols::ControlMethod::TimedEvents::clsid() );

    if ( it != m->end() ) {
        
        if ( auto tt = std::make_shared< adcontrols::ControlMethod::TimedEvents >() ) {
            if ( it->get( *it, *tt ) ) {
                ttable_ = std::move( tt );
                nextIt_ = ttable_->begin();
            }
        }
    }
    
//#ifndef NDEBUG
    if ( ! ttable_ )
        ADDEBUG() << "############# No time event table ##############";
    else {
        ADDEBUG() << "--------------------- time event table ---------->";
        std::for_each( ttable_->begin(), ttable_->end(), []( const auto& e ){
                ADDEBUG() << "### " << boost::format( "%8.2f\t%s\t%s" ) % e.time() % e.item_name() % e.toString( e.value() );
            });
        ADDEBUG() << "<-------------------- time event table -----------";
    }
//#endif
}

void
time_event_processor::setMethodTime( double t )
{
    methodTime_ = t;
}

void
time_event_processor::action_start()
{
    deadline_timer_.cancel();

    tp_started_ = this_clock::now(); // timer will start based on this tp

    preparing_for_run();
}

void
time_event_processor::action_stop()
{
    deadline_timer_.cancel();
}

void
time_event_processor::action_inject( const this_clock::time_point& tp )
{
    tp_inject_ = tp;
    exec_steps();
}

bool
time_event_processor::preparing_for_run()
{
    if ( ! ttable_ )
        return false;

    nextIt_ = ttable_->begin();

    if ( ttable_ && ( nextIt_ != ttable_->end() ) ) {

        auto begin = nextIt_;

        while ( nextIt_ != ttable_->end() && nextIt_->time() < 0.0 )
            ++nextIt_;

        if ( nextIt_ != begin )
            task::instance()->time_event_trigger( ttable_, begin, nextIt_ );
    }

#if ! defined NDEBUG && 0
    ADDEBUG() << "preparing for run next time: " << ( ( ttable_ && nextIt_ != ttable_->end() ) ? nextIt_->time() : -1 );
#endif

    return true;
}

bool
time_event_processor::exec_steps()
{
    bool hasNext( false );

    if ( ttable_ && ( nextIt_ != ttable_->end() ) ) {
        auto elapsed_time = std::chrono::duration_cast< std::chrono::seconds >( std::chrono::duration<double>( this_clock::now() - tp_inject_ ) ).count();

        auto begin = nextIt_++;

        while ( nextIt_ != ttable_->end() && nextIt_->time() < elapsed_time )
            ++nextIt_;

        task::instance()->time_event_trigger( ttable_, begin, nextIt_ );

        if ( nextIt_ != ttable_->end() ) {
            
            auto duration = std::chrono::duration_cast< std::chrono::microseconds >( std::chrono::duration< double >( nextIt_->time() ) );
            this_clock::time_point next_time_point = tp_inject_ + duration;

            deadline_timer_.expires_at( next_time_point );
            deadline_timer_.async_wait( [&]( const boost::system::error_code& ec ){ on_timer( ec ); } );
        }

    }

    return hasNext;
}

void
time_event_processor::on_timer( const boost::system::error_code& ec )
{
    if ( ! ec ) {
        if ( ttable_ && nextIt_ != ttable_->end() ) {
            exec_steps();
        }
    }
}
