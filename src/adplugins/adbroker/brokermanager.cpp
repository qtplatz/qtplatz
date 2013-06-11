// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include "brokermanager.hpp"
#include "task.hpp"

using namespace adbroker;

// BrokerManager * BrokerManager::instance_ = 0;
bool BrokerManager::initialized_ = false;

namespace adbroker {
    template<> Task * BrokerManager::get<Task>()
    { 
        return pTask_;
    }
}

BrokerManager::~BrokerManager()
{
    //delete pTask_;
}

BrokerManager::BrokerManager() : pTask_(0)
{
    initialized_ = true;
    pTask_ = new Task(5);
    initialize();
}

bool
BrokerManager::initialize()
{
    pTask_->task_open();
    return true;
}

// static
void
BrokerManager::terminate()
{
    if ( initialized_ )
        singleton::BrokerManager::instance()->pTask_->task_close();
}
