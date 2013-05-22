// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
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

#include "timeutil.hpp"

using namespace adcontrols;

seconds_t::seconds_t( const seconds_t& t ) : seconds( t.seconds )
{
}

seconds_t::seconds_t( const minutes_t& t ) : seconds( timeutil::toSeconds( t ) )
{
}

minutes_t::minutes_t( const minutes_t& t ) : minutes( t.minutes )
{
}

minutes_t::minutes_t( const seconds_t& t ) : minutes( timeutil::toMinutes( t ).minutes )
{
}

seconds_t
timeutil::toSeconds( const minutes_t& m )
{
    return m.minutes * 60.0;
}

minutes_t
timeutil::toMinutes( const seconds_t& s )
{
    return s.seconds / 60.0;
}

std::pair<double, double>
timeutil::toMinutes( const std::pair<seconds_t, seconds_t>& pair )
{
    return std::make_pair( pair.first.seconds / 60.0, pair.second.seconds / 60.0 );
}
