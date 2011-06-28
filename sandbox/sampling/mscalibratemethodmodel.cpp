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

#include "mscalibratemethodmodel.hpp"

MSCalibrateMethodModel::MSCalibrateMethodModel(QObject *parent) :
    QObject(parent)
{
}

unsigned int
MSCalibrateMethodModel::polynomialDegree() const
{
    return method_.polynomialDegree();
}

void
MSCalibrateMethodModel::polynomialDegree( unsigned int t )
{
    method_.polynomialDegree( t );
}


double
MSCalibrateMethodModel::massToleranceDa() const
{
    return method_.massToleranceDa();
}

void
MSCalibrateMethodModel::massToleranceDa( double t )
{
    method_.massToleranceDa( t );
}

double
MSCalibrateMethodModel::minimumRAPercent() const
{
    return method_.minimumRAPercent();
}

void
MSCalibrateMethodModel::minimumRAPercent( double t )
{
    method_.minimumRAPercent( t );
}

double
MSCalibrateMethodModel::lowMass() const
{
    return method_.lowMass();
}

void
MSCalibrateMethodModel::lowMass( double t )
{
    method_.lowMass( t );
}

double
MSCalibrateMethodModel::highMass() const
{
    return method_.highMass();
}

void
MSCalibrateMethodModel::highMass( double t )
{
   method_.highMass( t );
}

//const MSReferenceDefns& refDefns() const;
//void refDefns( const MSReferenceDefns& );

const adcontrols::MSReferences&
MSCalibrateMethodModel::references() const
{
    return method_.references();
}

void
MSCalibrateMethodModel::references( const adcontrols::MSReferences& t )
{
    method_.references( t );
}
