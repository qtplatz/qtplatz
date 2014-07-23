/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#pragma once

#include <string>
#include <ctime>
#include <chrono>

namespace boost { namespace gregorian { class date; } }

namespace adportable {

    class date_string {
    public:
        static std::string string( const boost::gregorian::date& dt, const char * fmt = "%Y-%m-%d" );
        static std::string utc_to_localtime_string( time_t utc, unsigned usec, bool add_utc_offset = false );
        static std::string logformat( const std::chrono::system_clock::time_point& tp, bool add_utc_offset = false );
    };

}

