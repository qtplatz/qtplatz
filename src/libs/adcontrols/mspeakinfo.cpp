/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
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
#include <adportable/debug.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>

using namespace adcontrols;

MSPeakInfo::MSPeakInfo( int mode ) : mode_( mode )
{
}

MSPeakInfo::MSPeakInfo( const MSPeakInfo& t ) : vec_( t.vec_ )
                                              , mode_( t.mode_ )
                                              , protocolId_( t.protocolId_ )
                                              , nProtocols_( t.nProtocols_ )
{
}

MSPeakInfo *
MSPeakInfo::findProtocol( int32_t proto )
{
    if ( protocolId_ == proto )
        return this;

    auto it = std::find_if( siblings_.begin(), siblings_.end(), [=]( const MSPeakInfo& p ){ return p.protocolId() == proto; } );
    if ( it != siblings_.end() )
        return &(*it);
    return 0;
}

const MSPeakInfo *
MSPeakInfo::findProtocol( int32_t proto ) const
{
    if ( protocolId_ == proto )
        return this;

    auto it = std::find_if( siblings_.begin(), siblings_.end(), [=]( const MSPeakInfo& p ){ return p.protocolId() == proto; } );
    if ( it != siblings_.end() )
        return &(*it);
    return 0;
}

void
MSPeakInfo::clearSegments()
{
    siblings_.clear();
}

int32_t
MSPeakInfo::protocolId() const
{
    return protocolId_;
}

int32_t
MSPeakInfo::nProtocols() const
{
    return nProtocols_;
}

void
MSPeakInfo::setProtocol( int32_t id, int32_t n )
{
    protocolId_ = id;
    nProtocols_ = n;
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
MSPeakInfo::setMode( int v )
{
    mode_ = v;
}

MSPeakInfo&
MSPeakInfo::operator << ( const MSPeakInfoItem& item )
{
    vec_.emplace_back( item );
    return *this;
}

MSPeakInfo&
MSPeakInfo::operator << ( MSPeakInfoItem&& item )
{
    vec_.emplace_back( item );
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

bool
MSPeakInfo::trim( MSPeakInfo& clone, const std::pair<double, double>& range ) const
{
    // make very-clean
    clone.clear();
    clone.clearSegments();

    auto itFirst = std::lower_bound( vec_.begin(), vec_.end(), range.first, [](const MSPeakInfoItem& a, double mass){ return a.mass() < mass; } );
    if ( itFirst == vec_.end() )
        return false;

    if ( itFirst != vec_.begin() )
        --itFirst;

    size_t idx = std::distance( vec_.begin(), itFirst );

    auto itSecond = std::lower_bound( vec_.begin(), vec_.end(), range.second, [](const MSPeakInfoItem& a, double mass){ return a.mass() < mass; } );
    std::for_each( itFirst, itSecond, [&clone, idx] ( const MSPeakInfoItem& item ){ clone << item; } );

    return true;
}

const MSPeakInfoItem&
MSPeakInfo::front() const
{
    return vec_.front();
}

const MSPeakInfoItem&
MSPeakInfo::back() const
{
    return vec_.back();
}

//static
std::pair< MSPeakInfo::const_iterator, MSPeakInfo::const_iterator >
MSPeakInfo::find_range( const MSPeakInfo& pki, double left, double right, bool isTime )
{
    if ( isTime ) {
        if ( pki.front().time() < right && left < pki.back().time() ) {
            auto first = std::lower_bound( pki.begin(), pki.end(), left, [] ( const MSPeakInfoItem& a, double t ) { return a.time() < t; } );
            auto last = std::lower_bound( pki.begin(), pki.end(), right, [] ( const MSPeakInfoItem& a, double t ) { return a.time() < t; } );
            return std::make_pair( first, last );
        }
    } else {
        if ( pki.front().mass() < right && left < pki.back().mass() ) {
            auto first = std::lower_bound( pki.begin(), pki.end(), left, [] ( const MSPeakInfoItem& a, double m ) { return a.mass() < m; } );
            auto last = std::lower_bound( pki.begin(), pki.end(), right, [] ( const MSPeakInfoItem& a, double m ) { return a.mass() < m; } );
            return std::make_pair( first, last );
        }
    }
    return std::make_pair( pki.end(), pki.end() );
}

//static
MSPeakInfo::const_iterator
MSPeakInfo::max_element( const MSPeakInfo& pki, double left, double right, bool isTime )
{
    if ( pki.size() > 0 ) {
        auto pair = find_range( pki, left, right, isTime );

        if ( pair.first != pki.end() ) {
            auto it =
                std::max_element( pair.first, pair.second, []( const MSPeakInfoItem& a, const MSPeakInfoItem& b ){ return a.height() < b.height(); } );
            return it;
        }
    }
    return pki.end();
}
