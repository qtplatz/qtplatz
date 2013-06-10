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
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// MS-Cheminformatics LLC / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <ace/Event_Handler.h>
#include <string>

namespace acewrapper {
	class McastHandler;
}

class ACE_Time_Value;

class mcast_handler {
	std::string ior_;
public:
	~mcast_handler(void);
	mcast_handler(void);

	const std::string & ior() { return ior_; }
	void ior( const std::string& ior ) { ior_ = ior; }

	int handle_input( acewrapper::McastHandler&, ACE_HANDLE );
    int handle_timeout( const ACE_Time_Value&, const void * );
    int handle_close( ACE_HANDLE, ACE_Reactor_Mask );
};
