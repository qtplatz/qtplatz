// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#ifndef REACTORTHREAD_H
#define REACTORTHREAD_H

#include <ace/Reactor.h>

namespace acewrapper {

    class ReactorThread {
	public:
        ~ReactorThread();
        ReactorThread();

        ACE_Reactor * get_reactor();

        bool spawn();
        bool end_reactor_event_loop();
        bool join();

    private:
        static void * thread_entry( void * me );
        void run_event_loop();
        ACE_Reactor * reactor_;
        ACE_thread_t t_handle_;
    };
}

#endif // REACTORTHREAD_H
