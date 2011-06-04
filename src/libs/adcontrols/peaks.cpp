// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#include "peaks.hpp"
#include "peak.hpp"
#include "baseline.hpp"
#include "baselines.hpp"

using namespace adcontrols;

Peaks::~Peaks()
{
}

Peaks::Peaks()
{
}

Peaks::Peaks( const Peaks& t ) : peaks_( t.peaks_ )
{
}

void
Peaks::add( const Peak& pk )
{
    peaks_.push_back( pk );
}

///////////////

Peaks::vector_type::const_iterator
Peaks::find_first_peak( const Baseline& bs ) const
{
    for ( vector_type::const_iterator it = begin(); it != end(); ++it ) {
        if ( bs.startTime() <= it->startTime() && it->endTime() <= bs.stopTime() )
            return it;
    }
    return end();
}

Peaks::vector_type::iterator
Peaks::find_first_peak( const Baseline& bs )
{
    for ( vector_type::iterator it = begin(); it != end(); ++it ) {
        if ( bs.startTime() <= it->startTime() && it->endTime() <= bs.stopTime() )
            return it;
    }
    return end();
}