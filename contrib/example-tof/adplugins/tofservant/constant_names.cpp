/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
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

#include "constant_names.hpp"

using namespace tofservant;

constant_names::constant_names()
{
}

const char *
constant_names::name( constants::SESSION_COMMAND cmd )
{
    using namespace tofservant::constants;
    switch ( cmd ) {
    case SESSION_SHELL:               return "SESSION_SHELL";
    case SESSION_INITIALIZE:          return "SESSION_INITIALIZE";
    case SESSION_INIT_RUN:            return "SESSION_INIT_RUN";
    case SESSION_START_RUN:           return "SESSION_START_RUN";
    case SESSION_SUSPEND_RUN:         return "SESSION_SUSPEND_RUN";        
    case SESSION_RESUME_RUN:          return "SESSION_RESUME_RUN";         
    case SESSION_STOP_RUN:            return "SESSION_STOP_RUN";           
    case SESSION_SHUTDOWN:            return "SESSION_SHUTDOWN";           
    case SESSION_SENDTO_DEVICE:       return "SESSION_SENDTO_DEVICE";      
    case SESSION_QUERY_DEVICE:        return "SESSION_QUERY_DEVICE";       
    case SESSION_ASYNC_COMMAND_BEGIN: return "SESSION_ASYNC_COMMAND_BEGIN";
    case SESSION_QUERY_DEVICE_DATA:   return "SESSION_QUERY_DEVICE_DATA";  
    default:                          
        return "";
    }    
}
