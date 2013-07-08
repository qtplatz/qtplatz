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

#include "taskmanager.hpp"
#include "task.hpp"
#include "message.hpp"
#include "constants.hpp"
#include "marshal.hpp"
#include <boost/noncopyable.hpp>
#include <acewrapper/mutex.hpp>
#include <acewrapper/reactorthread.hpp>
#include <acewrapper/timerhandler.hpp>
#include <acewrapper/eventhandler.hpp>
#include <ace/Thread_Manager.h>
#include <ace/Reactor.h>

//////////////////////////

using namespace adcontroller;

iTaskManager * iTaskManager::instance_ = 0;
std::mutex iTaskManager::mutex_;

///////////////////////////////////////////////////////////////////

iTaskManager::~iTaskManager()
{
    manager_terminate();
    ACE_Thread_Manager::instance()->wait();
}

iTaskManager::iTaskManager() : pTask_( new iTask )
{
}

// static
iTaskManager *
iTaskManager::instance()
{
	if ( instance_ == 0 ) {
		std::lock_guard< std::mutex > lock( mutex_ );
		if ( instance_ == 0 )
			instance_ = new iTaskManager;
	}
    return instance_;
}

// static
iTask&
iTaskManager::task()
{
    return *( instance()->pTask_ );
}

bool
iTaskManager::manager_initialize()
{
	pTask_->open();
	return true;
}

void
iTaskManager::manager_terminate()
{
    pTask_->close();
}
