// -*- C++ -*-
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

#include "peak.hpp"
#include "peaks.hpp"
#include "baseline.hpp"
#include "baselines.hpp"
#include <adportable/json_helper.hpp>
#include <adportable/json/extract.hpp>
#include <boost/json.hpp>
#include <algorithm>

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

Peaks&
Peaks::operator = ( const Peaks& t )
{
    peaks_ = t.peaks_;
    return *this;
}

void
Peaks::add( const Peak& pk )
{
    peaks_.emplace_back( pk );
}

Peaks::value_type&
Peaks::emplace_back( Peak&& t )
{
    return peaks_.emplace_back( std::move( t ) );
}

///////////////
Peaks::vector_type::const_iterator
Peaks::find_peakId( int32_t peakid ) const
{
    return std::find_if( peaks_.begin(), peaks_.end(), [peakid] ( const value_type& pk ) { return pk.peakId() == peakid; } );
}

Peaks::vector_type::iterator
Peaks::find_peakId( int32_t peakid )
{
    return std::find_if( peaks_.begin(), peaks_.end(), [peakid] ( value_type& pk ) { return pk.peakId() == peakid; } );
}

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

double
Peaks::areaTotal() const
{
	return areaTotal_;
}

void
Peaks::areaTotal( double v )
{
	areaTotal_ = v;
}

double
Peaks::heightTotal() const
{
	return heightTotal_;
}

void
Peaks::heightTotal( double v )
{
	heightTotal_ = v;
}

double
Peaks::noiseLevel() const
{
	return noiseLevel_;
}

void
Peaks::noiseLevel( double v )
{
   noiseLevel_ = v;
}

namespace adcontrols {

    void
    tag_invoke( boost::json::value_from_tag, boost::json::value& jv, const Peaks& t )
    {
        jv = {{ "peaks", t.peaks_ }
              ,{ "areaTotal", t.areaTotal_ }
              ,{ "heightTotal", t.heightTotal_ }
              ,{ "noiseLevel",  t.noiseLevel_ }
        };
    }

    Peaks
    tag_invoke( boost::json::value_to_tag< Peaks >&, const boost::json::value& jv )
    {
        Peaks t;
        using namespace adportable::json;

        if ( jv.is_object() ) {
            auto obj = jv.as_object();
            extract( obj,     t.peaks_,                 "peaks"        );
            extract( obj,     t.areaTotal_,             "areaTotal"    );
            extract( obj,     t.heightTotal_,           "heightTotal"  );
            extract( obj,     t.noiseLevel_,            "noiseLevel"   );
        }

        return t;
    }


}
