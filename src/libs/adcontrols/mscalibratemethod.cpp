// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
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

#include "mscalibratemethod.h"
#include "msreferencedefns.h"
#include "msreferences.h"
#include "msreference.h"

using namespace adcontrols;

MSCalibrateMethod::~MSCalibrateMethod()
{
}

MSCalibrateMethod::MSCalibrateMethod() : polynomialDegree_(1)
                                       , massToleranceDa_( 0.2 )
                                       , minimumRAPercent_( 2.0 )
                                       , lowMass_( 100.0 )
                                       , highMass_( 1000.0 )
                                       , refDefns_( new MSReferenceDefns() ) 
                                       , references_( new MSReferences ) 
{
}

MSCalibrateMethod::MSCalibrateMethod( const MSCalibrateMethod& t ) : polynomialDegree_( t.polynomialDegree_ )
                                                                   , massToleranceDa_( t.massToleranceDa_ )
                                                                   , minimumRAPercent_( t.minimumRAPercent_ )
                                                                   , lowMass_( t.lowMass_ )
                                                                   , highMass_( t.highMass_ )
                                                                   , refDefns_( new MSReferenceDefns( *t.refDefns_ ) )
                                                                   , references_( new MSReferences( *t.references_ ) )  
{
}

MSCalibrateMethod&
MSCalibrateMethod::operator = ( const MSCalibrateMethod& t )
{
    polynomialDegree_ = t.polynomialDegree_;
    massToleranceDa_ = t.massToleranceDa_;
    minimumRAPercent_ = t.minimumRAPercent_;
    lowMass_ = t.lowMass_;
    highMass_ = t.highMass_;
    (*refDefns_) = (*t.refDefns_);
    (*references_) = (*t.references_);
    return *this;
}

unsigned int
MSCalibrateMethod::polynomialDegree() const
{
    return polynomialDegree_;
}

void
MSCalibrateMethod::polynomialDegree( unsigned int value )
{
    polynomialDegree_ = value;
}

double
MSCalibrateMethod::massToleranceDa() const
{
    return massToleranceDa_;
}

void
MSCalibrateMethod::massToleranceDa( double value )
{
    massToleranceDa_ = value;
}

double
MSCalibrateMethod::minimumRAPercent() const
{
    return minimumRAPercent_;
}

void
MSCalibrateMethod::minimumRAPercent( double value )
{
    minimumRAPercent_ = value;
}

double
MSCalibrateMethod::lowMass() const
{
    return lowMass_;
}

void
MSCalibrateMethod::lowMass( double value )
{
    lowMass_ = value;
}

double
MSCalibrateMethod::highMass() const
{
    return highMass_;
}

void
MSCalibrateMethod::highMass( double value )
{
    highMass_ = value;
}

const MSReferences&
MSCalibrateMethod::references() const
{
    return *references_;
}

void
MSCalibrateMethod::references( const MSReferences& ref )
{
    *references_ = ref;
}