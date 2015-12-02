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

#include "constants.hpp"
#include <boost/signals2.hpp>
#include <functional>
#include <memory>

namespace adicontroller {

    class SampleProcessor;

    class task {
        
        ~task();
        task();

    public:
        static task * instance();

        typedef std::function< void( Instrument::eInstEvent ) > signal_inst_events_t;
        typedef std::function< void( uint32_t ) > signal_fsm_action_t;

        void initialize();
        void finalize();
        boost::signals2::connection connect( signal_inst_events_t );
        boost::signals2::connection connect( signal_fsm_action_t );

    private:
        friend std::unique_ptr< task >::deleter_type;
        class impl;
        std::unique_ptr< impl > impl_;
    };

} // namespace adcontroller

