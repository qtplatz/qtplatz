// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC
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
#include <chrono>
#include <boost/json/fwd.hpp>

namespace adnetcdf {

    typedef std::chrono::time_point< std::chrono::system_clock, std::chrono::nanoseconds > time_point_type;

    struct time_stamp_parser {
        time_point_type operator()(const boost::json::value&, const std::string& path ) const;
    };

    struct iso8601 {
        std::string operator()( time_point_type&& tp ) const;
    };

}
