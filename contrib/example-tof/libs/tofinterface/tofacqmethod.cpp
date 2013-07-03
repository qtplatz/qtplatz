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

#include "tofacqmethod.hpp"
#include "protocolids.hpp"

using namespace tofinterface;

tofAcqMethod::tofAcqMethod() : protocolId_( Constants::SMTD )
{
}

tofAcqMethod::tofAcqMethod( const tofAcqMethod& t ) : protocolId_( t.protocolId_ )
						    , methodId_( t.methodId_ )
						    , acqSegments_( t.acqSegments_ )
{
}

uint32_t
tofAcqMethod::protocolId() const
{
    return protocolId_;
}

void
tofAcqMethod::methodId( uint32_t v )
{
    methodId_ = v;
}

uint32_t
tofAcqMethod::methodId() const
{
    return methodId_;
}

uint16_t
tofAcqMethod::number_of_profiles() const
{
    return acqSegments_.size();
}

//-------------------------------------------------------------------------------
tofAcqMethod::acqSegment::acqSegment() : startIndex_( 0 )
				       , numberOfAverage_( 0 )
				       , waitTime_( 0 )
				       , numberOfSamples_( 0 )
				       , stepOf_( 0 )
{
}

tofAcqMethod::acqSegment::acqSegment( const acqSegment& t ) : startIndex_( t.startIndex_ )
							    , numberOfAverage_( t.numberOfAverage_ )
							    , waitTime_( t.waitTime_ )
							    , numberOfSamples_( t.numberOfSamples_ )
							    , stepOf_( t.stepOf_ )
{
}

uint16_t
tofAcqMethod::acqSegment::startIndex() const
{
    return startIndex_;
}

uint16_t
tofAcqMethod::acqSegment::numberOfAverage() const
{
    return numberOfAverage_;
}

uint32_t
tofAcqMethod::acqSegment::waitTime() const
{
    return waitTime_;
}

uint16_t
tofAcqMethod::acqSegment::numberOfSamples() const
{
    return numberOfSamples_;
}

uint16_t
tofAcqMethod::acqSegment::stepOf() const
{
    return stepOf_;
}

void
tofAcqMethod::acqSegment::startIndex( uint16_t v )
{
    startIndex_ = v;
}

void
tofAcqMethod::acqSegment::numberOfAverage( uint16_t v )
{
    numberOfAverage_ = v;
}

void
tofAcqMethod::acqSegment::waitTime( uint32_t v )
{
    waitTime_ = v;
}

void
tofAcqMethod::acqSegment::numberOfSamples( uint16_t v )
{
    numberOfSamples_ = v;
}

void
tofAcqMethod::acqSegment::stepOf( uint16_t v )
{
    stepOf_ = v;
}
