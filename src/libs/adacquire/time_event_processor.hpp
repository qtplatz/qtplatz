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

#pragma once

#include <adcontrols/controlmethod.hpp>
#include <boost/signals2.hpp>
#include <boost/asio.hpp>
#include <chrono>
#include <deque>
#include <memory>
#include <mutex>
#include <ctime>

namespace adacquire {

    enum time_event_state {
        time_event_stopped
        , time_event_preparing_for_run
        , time_event_running
        , time_event_post_running
    };
        
    class time_event_processor {
        // non copyable
        time_event_processor( const time_event_processor& ) = delete;
        time_event_processor& operator = ( const time_event_processor& ) = delete;        

    public:
        typedef std::chrono::system_clock this_clock;

        time_event_processor();
        ~time_event_processor();

        void setControlMethod( std::shared_ptr< const adcontrols::ControlMethod::Method >, double methodTime );
        std::shared_ptr< const adcontrols::ControlMethod::Method > controlMethod() const;
        void setMethodTime( double );
        double methodTime() const;
        
        void action_start(); // will fire initial condition
        void action_inject( const this_clock::time_point& );        
        void action_stop();
        
    private:
        typedef boost::asio::basic_waitable_timer< this_clock > this_timer;
        
        double methodTime_;

        this_clock::time_point tp_started_;
        this_clock::time_point tp_inject_;
        this_timer deadline_timer_;

        std::shared_ptr< const adcontrols::ControlMethod::Method > method_;
        std::shared_ptr< const adcontrols::ControlMethod::TimedEvents > ttable_;
        adcontrols::ControlMethod::const_time_event_iterator nextIt_;
        
        bool exec_steps();
        bool preparing_for_run();
        void on_timer( const boost::system::error_code& );
    };
}
