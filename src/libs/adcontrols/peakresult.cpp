/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
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

#include <compiler/diagnostic_push.h>
#include <compiler/disable_unused_parameter.h>

#include "peakresult.hpp"
#include "baselines.hpp"
#include "baseline.hpp"
#include "peaks.hpp"
#include "peak.hpp"
#include <boost/serialization/string.hpp> 
#include <boost/serialization/vector.hpp>
#include <boost/serialization/base_object.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>

#include <compiler/diagnostic_pop.h>

using namespace adcontrols;

PeakResult::~PeakResult()
{
}

PeakResult::PeakResult() : baselines_( std::make_shared< Baselines >() )
                         , peaks_( std::make_shared< Peaks >() ) 
{
}

PeakResult::PeakResult( const PeakResult& t ) : baselines_( std::make_shared< Baselines >( t.baselines() ) )
                                              , peaks_( std::make_shared< Peaks >( t.peaks() ) ) 
{
}

PeakResult::PeakResult( const Baselines& bs
                        , const Peaks& pks ) : baselines_( std::make_shared< Baselines >( bs ) )
                                             , peaks_( std::make_shared< Peaks >( pks ) ) 
{
}

const Baselines& 
PeakResult::baselines() const
{
	return * baselines_;
}

Baselines&
PeakResult::baselines()
{
	return * baselines_;
}


const Peaks&
PeakResult::peaks() const
{
	return * peaks_;
}

Peaks&
PeakResult::peaks()
{
	return * peaks_;
}

// ----- static -----
bool
PeakResult::archive( std::ostream& os, const PeakResult& t )
{
    portable_binary_oarchive ar( os );
    ar << t;
    return true;
}

bool
PeakResult::restore( std::istream& is, PeakResult& t )
{
    portable_binary_iarchive ar( is );
    ar >> t;
    return true;
}
