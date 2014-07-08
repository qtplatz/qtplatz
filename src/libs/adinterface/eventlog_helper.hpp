// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#pragma once

#include <string>
#include <adportable/string.hpp>
#include <boost/format.hpp>
#include <compiler/diagnostic_push.h>
#include <compiler/disable_deprecated.h>
#include <adinterface/eventlogC.h>
#include <compiler/diagnostic_pop.h>

namespace adinterface {

    namespace EventLog {

        class LogMessageHelper {
        public:
            LogMessageHelper( const std::wstring& format = L""
                              , unsigned long pri = 0
                              , const std::wstring& msgId = L""
                              , const std::wstring& srcId = L"");
            LogMessageHelper( const LogMessageHelper& );

            LogMessageHelper& format( const std::wstring& );

            template<class T> LogMessageHelper& operator % (const T& t) {
                msg_.args.length( msg_.args.length() + 1 );
                msg_.args[ msg_.args.length() - 1 ]
                    = CORBA::wstring_dup( ( boost::wformat(L"%1%") % t ).str().c_str() );
                return *this;
            }
            inline ::EventLog::LogMessage & get() { return msg_; }
            static std::wstring toString( const ::EventLog::LogMessage& );
            
        private:
            ::EventLog::LogMessage msg_;
        };

        template<> LogMessageHelper& LogMessageHelper::operator % (const std::string& );
        
    }
}
