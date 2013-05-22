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

#include "baseline.hpp"

using namespace adcontrols;

Baseline::~Baseline()
{
}

Baseline::Baseline() : manuallyModified_( false ) 
                     , baseId_( 0 )
                     , startPos_( 0 )
                     , stopPos_( 0 )
                     , startHeight_( 0 )
                     , stopHeight_( 0 )
{
}

Baseline::Baseline( const Baseline& t ) : manuallyModified_( t.manuallyModified_ )
                                        , baseId_( t.baseId_ )
                                        , startPos_( t.startPos_ )
                                        , stopPos_( t.stopPos_ )
                                        , startHeight_( t.startHeight_ )
                                        , stopHeight_( t.stopHeight_ )
                                        , startTime_( t.startTime_ )
                                        , stopTime_( t.stopTime_ )  
{
}

long
Baseline::baseId() const
{
    return baseId_;
}

void
Baseline::baseId( long value )
{
    baseId_ = value;
}

long
Baseline::startPos() const
{
    return startPos_;
}

void
Baseline::startPos( long value )
{
    startPos_ = value;
}
       
long
Baseline::stopPos() const
{
    return stopPos_;
}

void
Baseline::stopPos( long value )
{
    stopPos_ = value;
}


bool
Baseline::isManuallyModified() const
{
    return manuallyModified_;
}

void
Baseline::manuallyModified( bool f )
{
    manuallyModified_ = f;
}

double
Baseline::startHeight() const
{
    return startHeight_;
}

double
Baseline::stopHeight() const
{
    return stopHeight_;
}

seconds_t
Baseline::startTime() const
{
    return startTime_;
}

seconds_t
Baseline::stopTime() const
{
    return stopTime_;
}
    
void
Baseline::startHeight( double value )
{
    startHeight_ = value;
}

void
Baseline::stopHeight( double value )
{
    stopHeight_ = value;
}

void
Baseline::startTime( const seconds_t& value )
{
    startTime_ = value;
}

void
Baseline::stopTime( const seconds_t& value )
{
    stopTime_ = value;
}

double
Baseline::height(int pos) const
{
    if ((stopPos_ - startPos_ ) == 0)
        return startHeight_;
    else
        return ((stopHeight_ - startHeight_) * ((double)(pos - startPos_) / (stopPos_ - startPos_))) + startHeight_;
}
