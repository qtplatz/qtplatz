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
