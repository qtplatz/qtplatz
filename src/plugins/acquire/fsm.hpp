/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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
#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/state_machine_def.hpp>
#include <boost/msm/front/functor_row.hpp>
#include <boost/msm/front/euml/common.hpp>
#include <boost/msm/front/euml/operator.hpp>
#include <boost/msm/front/euml/state_grammar.hpp>

namespace acquire {

    namespace fsm {

        // events
        namespace msm = boost::msm;
        namespace mpl = boost::mpl;
        using boost::msm::front::Row;
        using boost::msm::front::none;

        /* fsm::prepare event := start instrument with initial method/condition but no time events started.
         */
        struct prepare {};

        /* fsm::start event := run sample
         */
        struct start_run {};

        /* fsm::stop event := stop current sample (close writing file), and fetch next sample
         * spectral/time-trace data monitoring is keep running
         */
        struct stop {};

        /* fsm::inject event := trigger acquisition start; Usually this start data writing on file, 
         * and (more importantly) start timer for timed event process
         */
        struct inject {};

        enum fsmStateId { idEmpty, idPreparing, idWaiting, idRunning, idDormant };

        // front-end
        struct acquire_ : public msm::front::state_machine_def< acquire_ > {
            template< class Event, class FSM >  void on_etry( Event const&, FSM& ) { ADDEBUG() << "enterling: peripheral"; }
            template< class Event, class FSM >  void on_exit( Event const&, FSM& ) { ADDEBUG() << "leaving: peripheral"; }

            /* Empty method and sample
             */
            struct Empty : public msm::front::state<> {
                template< class Event, class FSM >  void on_etry( Event const&, FSM& ) { ADDEBUG() << "enterling: Empty"; }
                template< class Event, class FSM >  void on_exit( Event const&, FSM& ) { ADDEBUG() << "leaving: Empty"; }
            };

            /* Preparing state; we have a sample w/ method.  Equilibrating instrument(s) for the sample
             */
            struct Preparing : public msm::front::state<> {
                template< class Event, class FSM >  void on_etry( Event const&, FSM& ) { ADDEBUG() << "enterling: Preparing"; }
                template< class Event, class FSM >  void on_exit( Event const&, FSM& ) { ADDEBUG() << "leaving: Preparing"; }
            };

            /* Instrument is waiting for inject trigger; This is actually a part of running state
             */
            struct Waiting : public msm::front::state<> {
                template< class Event, class FSM >  void on_etry( Event const&, FSM& ) { ADDEBUG() << "enterling: WaitForContactClosure"; }
                template< class Event, class FSM >  void on_exit( Event const&, FSM& ) { ADDEBUG() << "leaving: WaitForContactClosure"; }
            };

            /* Sample is running -- data is recording on file, time is in progress for time event execution.
             */
            struct Running : public msm::front::state<> {
                template< class Event, class FSM >  void on_etry( Event const&, FSM& ) { ADDEBUG() << "enterling: Running"; }
                template< class Event, class FSM >  void on_exit( Event const&, FSM& ) { ADDEBUG() << "leaving: Running"; }
            };

            struct Dormant : public msm::front::state<> {
                // Method and sample has been completed.  Keep instrument running at the last step of method
                template< class Event, class FSM >  void on_etry( Event const&, FSM& ) { ADDEBUG() << "enterling: Dormant"; }
                template< class Event, class FSM >  void on_exit( Event const&, FSM& ) { ADDEBUG() << "leaving: Dormant"; }
            };

            void action( prepare const& ) {}
            void action( start_run const& ) {}
            void action( stop const& ) {}
            void action( inject const& ) {}

            typedef acquire_ p;

            // Transition table
            struct transition_table : mpl::vector<
                //    Start          Event         Next          Action				 Guard
                //  +--------------+-------------+-------------+---------------------+----------------------+
                // 0 = Empty
                  a_row < Empty,       prepare,    Preparing,    &p::action >
                , a_row < Empty,       start_run,  Waiting,      &p::action >

                // 1 = Preparing
                , a_row < Preparing,   start_run,  Waiting,      &p::action >
                , a_row < Preparing,   stop,       Dormant,      &p::action >
                , a_row < Preparing,   inject,     Running,      &p::action >

                // 2 = WaitingForContactClosure
                , a_row < Waiting,     inject,     Running,      &p::action >
                , a_row < Waiting,     stop,       Dormant,      &p::action >

                // 3 = Running
                , a_row < Running,     stop,       Dormant,      &p::action >

                // 4 = Dormant
                , a_row < Dormant,     stop,       Empty,        &p::action >
                , a_row < Dormant,     prepare,    Preparing,    &p::action >
                , a_row < Dormant,     start_run,  Waiting,      &p::action >
                > {};
            template<class FSM, class Event>
            void no_transition( Event const& e, FSM&, int state ) {
                ADDEBUG() << "no transition from state " << state << " on event " << typeid( e ).name();
            }
            typedef Empty initial_state;
        };
        
        // back-end
        typedef msm::back::state_machine< acquire_ > acquire;
        
    } // namespace fsm
}

