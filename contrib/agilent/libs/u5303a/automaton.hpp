/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
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
#include <adportable/debug.hpp>

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

namespace u5303a {

    namespace fsm {

        namespace msm = boost::msm;
        namespace mpl = boost::mpl;
        using boost::msm::front::Row;
        using boost::msm::front::none;

        // events
        struct Stop        {};
        struct Prepare     {};
        struct Initiate    {};
        struct TSRInitiate {};
        struct Continue    {};
        struct error_clear {};

        struct error_detected {
            error_detected(const std::string& name)  : name_(name)
                {}
            std::string name_;
        };

        struct handler;

        struct controller_ : public msm::front::state_machine_def< controller_ > {

            controller_( handler * p ) : handler_( p ) {}

            struct Error_tag {};
            typedef msm::front::euml::func_state< Error_tag > Error;

            //-----------------------------
            struct Stopped_Entry { 
                template <class Event, class FSM, class STATE> void operator()( Event const& evt, FSM& fsm, STATE& ) {
                }
            };
            struct Stopped_Exit { 
                template <class Event, class FSM, class STATE> void operator()( Event const& evt, FSM& fsm, STATE& ) {
                }
            };
            struct Stopped_tag {};
            typedef msm::front::euml::func_state< Stopped_tag, Stopped_Entry, Stopped_Exit> Stopped;

            //-----------------------------
            struct Preparing_Entry { 
                template <class Event, class FSM, class STATE> void operator()( Event const& evt, FSM& fsm, STATE& state ) {
                }
            };
            struct Preparing_Exit { 
                template <class Event, class FSM, class STATE> void operator()( Event const& evt, FSM& fsm, STATE& state ) {
                }
            };
            struct Preparing_tag {};
            typedef msm::front::euml::func_state< Preparing_tag, Preparing_Entry, Preparing_Exit> Preparing;
            
            struct ReadyToInitiate_Entry { 
                template <class Event, class FSM, class STATE> void operator()( Event const& evt, FSM& fsm, STATE& ) {
                    // nothing
                }
            };
            struct ReadyToInitiate_Exit { 
                template <class Event, class FSM, class STATE> void operator()( Event const& evt, FSM& fsm, STATE& ) {
                    // noting
                }
            };
            struct ReadyToInitiate_tag {};
            typedef msm::front::euml::func_state< Preparing_tag, Preparing_Entry, Preparing_Exit> ReadyToInitiate;
            

            //-----------------------------
            struct Running_Entry {
                template <class Event, class FSM, class STATE> void operator()( Event const& evt, FSM& fsm, STATE& ) {
                }
            };
            struct Running_Exit {
                template <class Event, class FSM, class STATE> void operator()( Event const& evt, FSM& fsm, STATE& ) {
                }
            };            
            struct Running_tag {};
            typedef msm::front::euml::func_state< Running_tag, Running_Entry, Running_Exit > Running;

            //-----------------------------
            struct TSRRunning_Entry {
                template <class Event, class FSM, class STATE> void operator()( Event const& evt, FSM& fsm, STATE& ) {
                }
            };
            struct TSRRunning_Exit {
                template <class Event, class FSM, class STATE> void operator()( Event const& evt, FSM& fsm, STATE& ) {
                }
            };
            struct TSRRunning_tag {};
            typedef msm::front::euml::func_state< TSRRunning_tag, TSRRunning_Entry, TSRRunning_Exit> TSRRunning;

            //-----------------------------
            typedef Stopped initial_state;

            //-----------------------------
            struct actPrepare {
                template <class EVT, class FSM, class SourceState, class TargetState >
                void operator()( EVT const& evt, FSM& fsm, SourceState&, TargetState& ) {
                    fsm.handler_->fsm_action_prepare();
                }
            };
            
            struct actInitiate {
                template <class EVT, class FSM, class SourceState, class TargetState >
                void operator()( EVT const& evt, FSM& fsm, SourceState&, TargetState& ) {
                    fsm.handler_->fsm_action_initiate();
                }
            };

            struct actTSRInitiate {
                template <class EVT, class FSM, class SourceState, class TargetState >
                void operator()( EVT const& evt, FSM& fsm, SourceState&, TargetState& ) {
                    fsm.handler_->fsm_action_TSR_initiate();
                }
            };

            struct actStop {
                template <class EVT, class FSM, class SourceState, class TargetState >
                void operator()( EVT const& evt, FSM& fsm, SourceState&, TargetState& ) {
                    fsm.handler_->fsm_action_stop();
                }
            };

            struct actTSRStop {
                template <class EVT, class FSM, class SourceState, class TargetState >
                void operator()( EVT const& evt, FSM& fsm, SourceState& prev, TargetState& ) {
                    fsm.handler_->fsm_action_TSR_stop();
                }
            };

            struct actContinue {
                template <class EVT, class FSM, class SourceState, class TargetState >
                void operator()( EVT const& evt, FSM& fsm, SourceState& prev, TargetState& ) {
                    fsm.handler_->fsm_action_continue();
                }
            };
            
            struct actTSRContinue {
                template <class EVT, class FSM, class SourceState, class TargetState >
                void operator()( EVT const& evt, FSM& fsm, SourceState& prev, TargetState& ) {
                    fsm.handler_->fsm_action_TSR_continue();
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

            // Transition table
            struct transition_table : mpl::vector<
                //    Start                Event             Next               Action				 Guard
                //  +-------------------+------------------+------------------+---------------------+----------------------+
                Row   < Stopped,         Stop,              Stopped,            none,             none >
                , Row < Stopped,         Prepare,           ReadyToInitiate,    actPrepare,       none >
                , Row < ReadyToInitiate, Stop,              Stopped,            none,             none >
                , Row < ReadyToInitiate, Initiate,          Running,            actInitiate,      none >
                , Row < ReadyToInitiate, TSRInitiate,       TSRRunning,         actTSRInitiate,   none >
                , Row < Running,         Stop,              Stopped,            actStop,          none >
                , Row < Running,         Continue,          Running,            actContinue,      none >
                , Row < Running,         Prepare,           ReadyToInitiate,    actPrepare,       none >
                , Row < TSRRunning,      Stop,              Stopped,            actTSRStop,       none >
                , Row < TSRRunning,      Continue,          TSRRunning,         actTSRContinue,   none >
                , Row < TSRRunning,      Prepare,           ReadyToInitiate,    actPrepare,       none >
                > {};

            // Replaces the default no-transition response.
            template <class FSM,class Event> void no_transition(Event const& e, FSM& fsm, int state)  {
                std::cout << "no transition from state " << state
                          << " on event " << typeid(e).name() << std::endl;
                }
            handler* handler_;
        };

        // handler
        struct handler {
            virtual void fsm_action_prepare() = 0;
            virtual void fsm_action_stop() = 0;
            virtual void fsm_action_TSR_stop() = 0;
            virtual void fsm_action_initiate() = 0;
            virtual void fsm_action_TSR_initiate() = 0;
            virtual void fsm_action_continue() = 0;
            virtual void fsm_action_TSR_continue() = 0;
        };

        // Pick a back-end
        typedef msm::back::state_machine< controller_ > controller;
    }
}
