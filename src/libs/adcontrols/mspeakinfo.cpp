/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "mspeakinfo.hpp"
#include "mspeakinfoitem.hpp"
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>

using namespace adcontrols;

MSPeakInfo::MSPeakInfo( int mode ) : mode_( mode )
{
}

MSPeakInfo::MSPeakInfo( const MSPeakInfo& t ) : vec_( t.vec_ )
                                              , mode_( t.mode_ )
{
}

void
MSPeakInfo::addSegment( const MSPeakInfo& sibling )
{
    siblings_.push_back( sibling );
}

MSPeakInfo&
MSPeakInfo::getSegment( size_t fcn )
{
    if ( siblings_.size() > fcn )
        return siblings_[ fcn ];
    throw std::out_of_range( "MSPeakInfo fragments subscript out of range" );
}

const MSPeakInfo&
MSPeakInfo::getSegment( size_t fcn ) const
{
    if ( siblings_.size() > fcn )
        return siblings_[ fcn ];
    throw std::out_of_range( "MSPeakInfo fragments subscript out of range" );
}

size_t 
MSPeakInfo::total_size() const
{
    size_t size = vec_.size();
    for ( const auto& sibling: siblings_ )
        size += sibling.size();
    return size;
}

size_t
MSPeakInfo::numSegments() const
{
    return siblings_.size();
}

int
MSPeakInfo::mode() const
{
    return mode_;
}

void
MSPeakInfo::mode( int v )
{
    mode_ = v;
}

MSPeakInfo&
MSPeakInfo::operator << ( const MSPeakInfoItem& item )
{
    vec_.push_back( item );
    return *this;
}

size_t
MSPeakInfo::size() const
{
    return vec_.size();
}

void
MSPeakInfo::clear()
{
    vec_.clear();
}


// static
bool
MSPeakInfo::archive( std::ostream& os, const MSPeakInfo& t )
{
    portable_binary_oarchive ar( os );
    ar << t;
    return true;
}

bool
MSPeakInfo::restore( std::istream& is, MSPeakInfo& t )
{
    portable_binary_iarchive ar( is );
    ar >> t;
    return true;
}
