// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "adicontroller_global.hpp"
#include "constants.hpp"
#include <boost/signals2.hpp>
#include <chrono>
#include <functional>
#include <memory>

namespace boost { namespace asio { class io_service; } }

namespace adcontrols { class SampleRun; namespace ControlMethod { class Method; } }

namespace adicontroller {

    class MasterObserver;
    class SampleProcessor;
    class SampleSequence;

    namespace SignalObserver { class DataWriter; }

    class ADICONTROLLERSHARED_EXPORT task {
        
        ~task();
        task();

    public:
        static task * instance();

        typedef void( inst_event_t )( Instrument::eInstEvent );
        typedef std::function< inst_event_t > signal_inst_events_t;

        typedef void( fsm_action_t )( Instrument::idFSMAction );
        typedef std::function< fsm_action_t > signal_fsm_action_t;

        typedef void( fsm_state_changed_t )(bool, int id_state, Instrument::eInstStatus);
        typedef std::function< fsm_state_changed_t > signal_fsm_state_changed_t;

        boost::signals2::connection connect_fsm_action( signal_fsm_action_t );
        boost::signals2::connection connect_fsm_state( signal_fsm_state_changed_t );
        boost::signals2::connection connect_inst_events( signal_inst_events_t );        
        
        void initialize();
        void finalize();
        boost::asio::io_service& io_service();

        const std::chrono::steady_clock::time_point& tp_uptime() const;
        const std::chrono::steady_clock::time_point& tp_inject() const;

        void post( std::shared_ptr< SampleProcessor >& );

        SampleSequence * sampleSequence();
        MasterObserver * masterObserver();

        // state control buttons -- corresponding to inst control buttuns
        void fsmStop();
        void fsmStart();
        void fsmReady(); // issued when all modules are ready to inject
        void fsmInject();
        void fsmErrorClear();

        adicontroller::Instrument::eInstStatus currentState() const;

        // prepare next sample strage
        void prepare_next_sample( std::shared_ptr< adcontrols::SampleRun >&, const adcontrols::ControlMethod::Method& );

        void handle_write( std::shared_ptr< adicontroller::SignalObserver::DataWriter > );

    private:
        friend std::unique_ptr< task >::deleter_type;
        class impl;
#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable:4251)
#endif
        std::unique_ptr< impl > impl_;
#if defined _MSC_VER
#pragma warning(pop)
#endif
    };

} // namespace adcontroller

