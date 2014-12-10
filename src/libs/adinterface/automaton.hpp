/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@scienceliaison.com
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

#include "instrument.hpp"
// back-end
#include <boost/msm/back/state_machine.hpp>

// front-end
#include <boost/msm/front/state_machine_def.hpp>

// functors
#include <boost/msm/front/functor_row.hpp>
#include <boost/msm/front/euml/common.hpp>

// And_ operator
#include <boost/msm/front/euml/operator.hpp>

// func_state, func_state_machine
#include <boost/msm/front/euml/state_grammar.hpp>

#include <iostream>
#include <functional>
#include <memory>

namespace adinterface {

    namespace fsm {

        namespace msm = boost::msm;
        namespace mpl = boost::mpl;
        using boost::msm::front::Row;
        using boost::msm::front::none;
        //using namespace msm::front;
        //using namespace msm::front::euml;

        // events
        struct onoff {};
        struct prepare {
            prepare( const std::string& method ) : method_( method ) 
                {}
            std::string method_;
        };
        struct run {};
        struct stop {};
        struct ready {};
        struct error_clear {};
        struct error_detected {
            error_detected(const std::string& name)  : name_(name)
                {}
            std::string name_;
        };

        // handler
        struct handler {
            virtual void handle_state( bool entering, adinterface::instrument::eInstStatus ) {}
            virtual void action_on( const onoff& ) { std::cout << "action_on" << std::endl; }
            virtual void action_prepare_for_run( const prepare& )  { std::cout << "action_prepare_for_run" << std::endl; }
            virtual void action_start_run( const run& )  { std::cout << "action_start_run" << std::endl; }
            virtual void action_stop_run( const stop& )  { std::cout << "action_stop_run" << std::endl; }
            virtual void action_off( const onoff& )  { std::cout << "action_off" << std::endl; }
            virtual void action_diagnostic( const onoff& )  { std::cout << "action_diagnostic" << std::endl; }
            virtual void action_error_detected( const error_detected& )  { std::cout << "action_error_detected" << std::endl; }
        };

        struct controller_ : public msm::front::state_machine_def< controller_ > {

            controller_( handler * p ) : handler_( p ) {}

            struct Off_Entry { 
                template <class Event, class FSM, class STATE> void operator()( Event const&, FSM& fsm, STATE& ) {
                    fsm.handler_->handle_state( true, adinterface::instrument::eOff );
                }
            };
            struct Off_Exit { 
                template <class Event, class FSM, class STATE> void operator()( Event const&, FSM& fsm, STATE& ) {
                    fsm.handler_->handle_state( false, adinterface::instrument::eOff );
                }
            };
            struct Off_tag {};
            typedef msm::front::euml::func_state< Off_tag, Off_Entry, Off_Exit> Off; 

            struct Error_tag {};
            typedef msm::front::euml::func_state< Error_tag > Error;

            struct StandBy_Entry {
                template <class Event, class FSM, class STATE> void operator()( Event const&, FSM& fsm, STATE& ) {
                    fsm.handler_->handle_state( true, adinterface::instrument::eStandBy );
                    std::cout << "entering: StandBy" << std::endl;
                }
            };
            struct StandBy_tag {};
            typedef msm::front::euml::func_state< StandBy_tag, StandBy_Entry > StandBy;

            struct Preparing_Entry {
                template <class Event, class FSM, class STATE> void operator()( Event const&, FSM& fsm, STATE& ) {
                    fsm.handler_->handle_state( true, adinterface::instrument::ePreparingForRun );
                    std::cout << "entering: Preparing" << std::endl;
                }
            };
            struct Preparing_Exit {
                template <class Event, class FSM, class STATE> void operator()( Event const&, FSM& fsm, STATE& ) {
                    fsm.handler_->handle_state( false, adinterface::instrument::ePreparingForRun );
                    std::cout << "leaving: Preparing" << std::endl;
                }
            };
            struct Preparing_tag {};
            typedef msm::front::euml::func_state< Preparing_tag, Preparing_Entry, Preparing_Exit> Preparing;

            struct ReadyForRun_Entry {
                template <class Event, class FSM, class STATE> void operator()( Event const&, FSM& fsm, STATE& ) {
                    fsm.handler_->handle_state( true, adinterface::instrument::eReadyForRun );
                    std::cout << "entering: ReadyForRun" << std::endl;
                }
            };
            struct ReadyForRun_Exit {
                template <class Event, class FSM, class STATE> void operator()( Event const&, FSM& fsm, STATE& ) {
                    fsm.handler_->handle_state( false, adinterface::instrument::eReadyForRun );
                    std::cout << "leaving: ReadyForRun" << std::endl;
                }
            };
            struct ReadyForRun_tag {};
            typedef msm::front::euml::func_state< ReadyForRun_tag, ReadyForRun_Entry, ReadyForRun_Exit > ReadyForRun;

            struct Running_Entry {
                template <class Event, class FSM, class STATE> void operator()( Event const&, FSM& fsm, STATE& ) {
                    fsm.handler_->handle_state( true, adinterface::instrument::eRunning );
                    std::cout << "entering: Running" << std::endl;
                }
            };
            struct Running_Exit {
                template <class Event,class FSM,class STATE> void operator()(Event const&,FSM& fsm,STATE& ) {
                    fsm.handler_->handle_state( false, adinterface::instrument::eRunning );
                    std::cout << "leaving: Running" << std::endl;
                }
            };
            struct Running_tag {};
            typedef msm::front::euml::func_state< Running_tag, Running_Entry, Running_Exit> Running;

            struct Dormant_tag {};
            typedef msm::front::euml::func_state< Dormant_tag > Dormant;

            struct Stopped_tag {};
            typedef msm::front::euml::func_state< Stopped_tag > Stopped;

            // the initial state of the player SM. Must be defined
            typedef Off initial_state;

            struct act_on {
                template <class EVT,class FSM,class SourceState,class TargetState>
                void operator()( EVT const& evt, FSM& fsm, SourceState&, TargetState& ) {
                    fsm.handler_->action_on( evt );
                }
            };

            struct act_prepare_for_run {
                template <class EVT,class FSM,class SourceState,class TargetState>
                void operator()( EVT const& evt, FSM& fsm, SourceState&, TargetState& ) {
                    fsm.handler_->action_prepare_for_run( evt );
                }
            };

            struct act_ready_for_run {
                template <class EVT,class FSM,class SourceState,class TargetState>
                void operator()( EVT const& evt, FSM& fsm, SourceState&, TargetState& ) {
                    fsm.handler_->action_ready_for_run( evt );
                }
            };

            struct act_start_run {
                template <class EVT,class FSM,class SourceState,class TargetState>
                void operator()( EVT const& evt, FSM& fsm, SourceState&, TargetState& ) {
                    fsm.handler_->action_start_run( evt );
                }
            };

            struct act_stop_run {
                template <class EVT,class FSM,class SourceState,class TargetState>
                void operator()( EVT const& evt, FSM& fsm, SourceState&, TargetState& ) {
                    fsm.handler_->action_stop_run( evt );
                }
            };

            struct act_off {
                template <class EVT,class FSM,class SourceState,class TargetState>
                void operator()( EVT const& evt, FSM& fsm, SourceState&, TargetState& ) {
                    fsm.handler_->action_off( evt );
                }
            };

            // guard conditions
            struct DummyGuard {
                template <class EVT,class FSM,class SourceState,class TargetState>
                bool operator()(EVT const& evt,FSM& fsm,SourceState& src,TargetState& tgt) {
                    return true;
                }
            };
            struct good_diag_result {
                template <class EVT,class FSM,class SourceState,class TargetState>
                bool operator()(EVT const& evt ,FSM&,SourceState& ,TargetState& ) {
                    return true;
                }
            };
            struct always_true {
                template <class EVT,class FSM,class SourceState,class TargetState>
                bool operator()(EVT const& evt ,FSM&,SourceState& ,TargetState& ) {             
                    return true;
                }
            };

            typedef controller_ p; // makes transition table cleaner

            // Transition table for player
            struct transition_table : mpl::vector<
                //    Start     Event         Next      Action				 Guard
                //  +---------+-------------+---------+---------------------+----------------------+
                Row   < Off,         onoff,    StandBy,     act_on,              DummyGuard >
                , Row < StandBy,     prepare,  Preparing,   act_prepare_for_run, none >
                , Row < Preparing,   ready,    ReadyForRun, none,                none >
                , Row < Preparing,   stop,     StandBy,     act_stop_run,        none >
                , Row < ReadyForRun, prepare,  Preparing,   act_prepare_for_run, none >
                , Row < ReadyForRun, run,      Running,     act_start_run,       none >
                , Row < ReadyForRun, stop,     StandBy,     act_stop_run,        none >
                , Row < Running,     stop,     ReadyForRun, act_stop_run,        none >
                > {};

            // Replaces the default no-transition response.
            template <class FSM,class Event> void no_transition(Event const& e, FSM& fsm, int state)  {
                std::cout << "no transition from state " << state
                          << " on event " << typeid(e).name() << std::endl;
                }
            handler* handler_;
        };

        // Pick a back-end
        typedef msm::back::state_machine< controller_ > controller;
    }
}
