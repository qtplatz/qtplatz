// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
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

#ifndef ADBROKERMANAGER_H
#define ADBROKERMANAGER_H

#include "adbroker_global.h"
# include <ace/Singleton.h>
# include <ace/Recursive_Thread_Mutex.h>

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

        BrokerSession * getBrokerSession();
        adwidgets::ElementIO& getElementIO();
  
    private:
	friend class ACE_Singleton<BrokerManager, ACE_Recursive_Thread_Mutex>;
        // friend class singleton::BrokerManager;
        static bool initialized_;
        Task * pTask_;
    };

    template<> Task * BrokerManager::get<Task>();

}

#endif // ADBROKERMANAGER_H
