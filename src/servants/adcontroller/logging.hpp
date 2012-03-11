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

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <adinterface/eventlog_helper.hpp>
#include <string>

namespace adcontroller {

    class Logging  {
        adinterface::EventLog::LogMessageHelper msg;
    public:
        ~Logging();
        Logging( const std::wstring& format = L""
                , ::EventLog::eMSGPRIORITY pri = ::EventLog::pri_DEBUG
                , const std::wstring& msgId = L""
                , const std::wstring& srcId = L"adcontroller");
        template<class T> Logging& operator % ( const T& t ) { msg % t; return *this; }
        void commit_to_broker();
        void commit_to_task();
    };

}

#endif // LOGGER_HPP
