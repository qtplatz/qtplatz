/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
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

#include "tofdata.hpp"
#include "tofacqmethod.hpp"
#include "protocolids.hpp"

// #include <boost/iostreams/device/array.hpp>
// #include <boost/iostreams/stream.hpp>
// #include <boost/archive/binary_iarchive.hpp>

using namespace tofinterface;

tofDATA::tofDATA() : protocolId_( Constants::DATA )
                   , sequenceNumber_( 0 )
                   , rtcTimeStamp_( 0 )
                   , clockTimeStamp_( 0 )
                   , wellKnownEvents_( 0 )
                   , methodId_( 0 )
                   , numberOfProfiles_( 0 )
{
}

tofDATA::tofDATA( const tofDATA& t ) : protocolId_( t.protocolId_ )
                                     , sequenceNumber_( t.sequenceNumber_ )
                                     , rtcTimeStamp_( t.rtcTimeStamp_ )
                                     , clockTimeStamp_( t.clockTimeStamp_ )
                                     , wellKnownEvents_( t.wellKnownEvents_ )
                                     , methodId_( t.methodId_ )
                                     , numberOfProfiles_( t.numberOfProfiles_ )
                                     , setpts_( t.setpts_ )
                                     , acts_( t.acts_ )
                                     , data_( t.data_ )
{
}

uint32_t
tofDATA::sequenceNumber() const
{
    return sequenceNumber_;
}

uint64_t
tofDATA::rtcTimeStamp() const
{
    return rtcTimeStamp_;
}

uint64_t
tofDATA::clockTimeStamp() const
{
    return clockTimeStamp_;
}

uint32_t
tofDATA::wellKnownEvents() const
{
    return wellKnownEvents_;
}

uint32_t
tofDATA::methodId() const
{
    return methodId_;
}

uint16_t
tofDATA::numberOfProfiles() const
{
    return data_.size();
}

void
tofDATA::numberOfProfiles( uint16_t v )
{
    data_.resize( v );
    numberOfProfiles_ = data_.size();
}

const tofStaticSetpts&
tofDATA::setpts() const
{
    return setpts_;
}

const tofStaticActs&
tofDATA::acts() const
{
    return acts_;
}

const std::vector< tofDATA::datum >&
tofDATA::data() const
{
    return data_;
}

std::vector< tofDATA::datum >&
tofDATA::data()
{
    return data_;
}

void
tofDATA::sequenceNumber( uint32_t v )
{
    sequenceNumber_ = v;
}

void
tofDATA::rtcTimeStamp( uint64_t v )
{
    rtcTimeStamp_ = v;
}

void
tofDATA::clockTimeStamp( uint64_t v )
{
    clockTimeStamp_ = v;
}

void
tofDATA::wellKnownEvents( uint32_t v )
{
    wellKnownEvents_ = v;
}

void
tofDATA::methodId( uint32_t v )
{
    methodId_ = v;
}

void
tofDATA::setpts( const tofStaticSetpts& v )
{
    setpts_ = v;
}

void
tofDATA::acts( const tofStaticActs& v )
{
    acts_ = v;
}

void
tofDATA::data( const std::vector< datum >& v )
{
    data_ = v;
}

//
tofDATA::datum::datum()
{
}

tofDATA::datum::datum( const datum& t ) : acqSegment_( t.acqSegment_ )
                                        , values_( t.values_ )
{
}

const tofAcqMethod::acqSegment& 
tofDATA::datum::acqSegment() const
{
    return acqSegment_;
}

void
tofDATA::datum::acqSegment( const tofAcqMethod::acqSegment& v )
{
    acqSegment_ = v;
}

const tofDATA::datum::vector_type& 
tofDATA::datum::values() const
{
    return values_;
}

tofDATA::datum::vector_type& 
tofDATA::datum::values()
{
    return values_;
}

////////////////////

