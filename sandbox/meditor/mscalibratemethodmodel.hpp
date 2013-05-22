// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
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

#ifndef MSCALIBRATEMETHODMODEL_HPP
#define MSCALIBRATEMETHODMODEL_HPP

#include <QObject>
#include <adcontrols/mscalibratemethod.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/msreferences.hpp>

class MSCalibrateMethodModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY( unsigned int polynomialDegree READ polynomialDegree WRITE polynomialDegree )
    Q_PROPERTY( double massToleranceDa READ massToleranceDa WRITE massToleranceDa )
    Q_PROPERTY( double minimumRAPercent READ minimumRAPercent WRITE minimumRAPercent )
    Q_PROPERTY( double lowMass READ lowMass WRITE lowMass )
    Q_PROPERTY( double highMass READ highMass WRITE highMass )
public:
    explicit MSCalibrateMethodModel(QObject *parent = 0);

    unsigned int polynomialDegree() const;
    void polynomialDegree( unsigned int );
    double massToleranceDa() const;
    void massToleranceDa( double );
    double minimumRAPercent() const;
    void minimumRAPercent( double );
    double lowMass() const;
    void lowMass( double );
    double highMass() const;
    void highMass( double );

    //const MSReferenceDefns& refDefns() const;
    //void refDefns( const MSReferenceDefns& );

    const adcontrols::MSReferences& references() const;
    void references( const adcontrols::MSReferences& );

signals:

public slots:

private:
    adcontrols::MSCalibrateMethod method_;
};

#endif // MSCALIBRATEMETHODMODEL_HPP
