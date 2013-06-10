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

#include <boost/noncopyable.hpp>
#include <ace/Event_Handler.h>

namespace acewrapper {

    template<class T> class EventHandler : public T
                                         , public ACE_Event_Handler {
    public:
        EventHandler() {}
        EventHandler(const T& t) : T(t) {}

        virtual ACE_HANDLE get_handle() const { return T::get_handle(); }
        virtual int handle_input(ACE_HANDLE h) { return T::handle_input(h); }
        virtual int handle_timeout( const ACE_Time_Value& tv, const void * arg = 0 ) {
            return T::handle_timeout(tv, arg);
        }
        virtual int handle_close( ACE_HANDLE handle, ACE_Reactor_Mask close_mask ) {
            return T::handle_close( handle, close_mask );
        }
    };

}


