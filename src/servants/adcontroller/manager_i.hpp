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

#pragma once

#if defined _MSC_VER
# pragma warning (disable: 4996)
#endif
#include "adinterface/brokerS.h"

#include "session_i.hpp"
#include <acewrapper/orbservant.hpp>
#include <map>
#include <string>
#include <boost/smart_ptr.hpp>

namespace adcontroller {

	class manager_i : public virtual POA_ControlServer::Manager {

	public:
		manager_i(void);
		~manager_i(void);

		void shutdown();
		ControlServer::Session_ptr getSession( const CORBA::WChar * );
	private:
		typedef std::map< std::wstring, boost::shared_ptr< adcontroller::session_i > > session_map_type;
		session_map_type session_list_;
	};

	namespace singleton {
		typedef ACE_Singleton< acewrapper::ORBServant< ::adcontroller::manager_i >, ACE_Recursive_Thread_Mutex > manager;
	}

}
