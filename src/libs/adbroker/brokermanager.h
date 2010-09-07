// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#ifndef ADBROKERMANAGER_H
#define ADBROKERMANAGER_H

#include "adbroker_global.h"

#pragma warning(disable:4996)
# include <ace/Singleton.h>
# include <ace/Recursive_Thread_Mutex.h>
#pragma warning(default:4996)

class BrokerSession;
class BrokerAccessToken;
class BrokerConfig;

namespace adwidgets {
    class ElementIO;
}

namespace adbroker {

    class Task;
    class BrokerManager;

	namespace singleton {
		typedef ACE_Singleton<BrokerManager, ACE_Recursive_Thread_Mutex> BrokerManager;
    }

    class ADBROKERSHARED_EXPORT BrokerManager {
        virtual ~BrokerManager();
        BrokerManager();
    public:
        // static BrokerManager * instance();
        
        bool initialize();
        static void terminate();

        template<class T> T* get();
        template<> Task * get<Task>() { return pTask_; }

        BrokerSession * getBrokerSession();
        adwidgets::ElementIO& getElementIO();
  
    private:
        friend singleton::BrokerManager;
        static bool initialized_;
        Task * pTask_;
    };

}

#endif // ADBROKERMANAGER_H
