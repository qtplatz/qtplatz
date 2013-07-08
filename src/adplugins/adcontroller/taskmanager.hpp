// This is a -*- C++ -*- header.
/**************************************************************************
 ** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
 ** Copyright (C) 2013 MS-Cheminformatics LLC
 *
 ** Contact: info@ms-cheminfo.com
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

#pragma once

#include <ace/Singleton.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <mutex>

class ACE_Reactor;

namespace acewrapper {
    class ReactorThread;
    template<class T> class EventHandler;
    class TimerHandler;
    template<class T> class TimerReceiver;
}

namespace adcontroller {

    class iTask;

    namespace internal {
        class TimeReceiver;
    }
    
    class iTaskManager {
    private:
        ~iTaskManager();
        iTaskManager();
        iTaskManager( const iTaskManager& );  /* not defined */
        static iTaskManager * instance_;
        
    public:  
        static iTaskManager * instance();
        static iTask& task();

        bool manager_initialize();
        void manager_terminate();
        
        inline std::mutex& mutex() { return mutex_; }
        ACE_Reactor * reactor();
        
    private:
        // friend class ACE_Singleton<iTaskManager, ACE_Recursive_Thread_Mutex>;
        friend class internal::TimeReceiver;
        int handle_timeout( const ACE_Time_Value&, const void * );
        
        static std::mutex mutex_;
        iTask * pTask_;
        acewrapper::ReactorThread * reactor_thread_;    
        acewrapper::EventHandler< acewrapper::TimerReceiver<internal::TimeReceiver> > * timerHandler_;
    };

}
