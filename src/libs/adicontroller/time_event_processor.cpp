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
#include <adportable/float.hpp>
#include <adportable/debug.hpp>

using namespace adicontroller;

time_event_processor::time_event_processor() : methodTime_( 60.0 )
                                             , tpNextStep_( 0 )
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
time_event_processor::setControlMethod( std::shared_ptr< const adcontrols::ControlMethod::Method > m )
{
    method_ = m;
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
    
    tpNextStep_ = 0.0;
    exec_steps();
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
time_event_processor::exec_steps()
{
    bool hasNext( false );
    
    auto it = std::lower_bound( method_->begin(), method_->end(), tpNextStep_
                                , [&]( const auto& a, const double b ){ return a.time() < b; });

    if ( it != method_->end() ) {
        auto begin = it;
        while ( it != method_->end()
                && adportable::compare<double>::approximatelyEqual( tpNextStep_, it->time() ) )
            ++it;

        task::instance()->time_event_trigger( 0, begin, it );

        if ( it != method_->end() ) {
            hasNext = true;
            tpNextStep_ = it->time();
        }

    } else {
        hasNext = false;
        tpNextStep_ = methodTime_;
    }

    auto duration = std::chrono::duration_cast< std::chrono::microseconds >( std::chrono::duration< double >( tpNextStep_ ) );
    this_clock::time_point next_time_point = tp_inject_ + duration;

    deadline_timer_.expires_at( next_time_point );
    deadline_timer_.async_wait( [&]( const boost::system::error_code& ec ){ on_timer( ec ); } );
                                                              
    return hasNext;
}

void
time_event_processor::on_timer( const boost::system::error_code& ec )
{
    if ( ! ec ) {
        if ( tpNextStep_ < methodTime_ ) {
            ADDEBUG() << "=================== on_timer =================== continue next: " << tpNextStep_;
            exec_steps();
        } else {
            ADDEBUG() << "=================== on_timer =================== prepare_for_run: ";
        }
    }
}
