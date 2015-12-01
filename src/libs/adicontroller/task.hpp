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

// #include <boost/noncopyable.hpp>
// #include <adportable/configuration.hpp>
// #include <adcontrols/controlmethod.hpp>
// #include <adcontrols/samplerun.hpp>

// #include <compiler/diagnostic_push.h>
// #include <compiler/disable_deprecated.h>
// #include <adinterface/controlserverC.h>
// #include <adinterface/signalobserverC.h>
// #include <compiler/diagnostic_pop.h>

#include <workaround/boost/asio.hpp>
#include <adportable/asio/thread.hpp>
#include <mutex>
#include <vector>
#include <deque>
#include <thread>

namespace adicontroller {

    class SampleProcessor;

    class task {
        
        ~task();
        task();
    public:
        static task * instance();

        void initialize();
        void finalize();

    private:
        friend std::unique_ptr< task >::deleter_type;
        class impl;
        std::unique_ptr< impl > impl_;
    };

} // namespace adcontroller

