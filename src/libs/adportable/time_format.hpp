/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2018 MS-Cheminformatics LLC
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

#include <boost/format.hpp>

namespace adportable {

	struct time_format {

        static std::string elapsed_time( int s ) {
            int days = s / (3600 * 24);
            int hh = ( s % (3600 * 24) ) / 3600;
            int mm = ( s / 60 ) % 60;
            int ss = s % 60;
            if ( days >= 2 )
                return ( boost::format( "%ddays %02d:%02d:%02d" ) % days % hh % mm % ss ).str();
            else if ( days >= 1 )
                return ( boost::format( "%dday %02d:%02d:%02d" ) % days % hh % mm % ss ).str();
            else if ( hh >= 0 )
                return ( boost::format( "%02d:%02d:%02d" ) % hh % mm % ss ).str();
            else
                return ( boost::format( "%02d:%02d" ) % mm % ss ).str();
        }
	};

}
