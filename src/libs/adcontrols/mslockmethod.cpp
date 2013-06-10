// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include "mslockmethod.hpp"
#include <adportable/is_equal.hpp>

using namespace adcontrols;

MSLockMethod::~MSLockMethod(void)
{
}

MSLockMethod::MSLockMethod() : toleranceMethod_( eToleranceMethodDa )
                             , massToleranceDa_(0.2)
                             , massTolerancePpm_(10.0)
                             , minimumPeakHeight_(10.0)
{
}

MSLockMethod::MSLockMethod( const MSLockMethod & t )
{
    operator=(t);
}

MSLockMethod&
MSLockMethod::operator = ( const MSLockMethod & rhs )
{
    toleranceMethod_ = rhs.toleranceMethod_;
    massToleranceDa_ = rhs.massToleranceDa_;
    massTolerancePpm_ = rhs.massTolerancePpm_;
    minimumPeakHeight_ = rhs.minimumPeakHeight_;
    refMassDefnsFullyQualifiedName_ = rhs.refMassDefnsFullyQualifiedName_;
    refMassDefnsXML_ = rhs.refMassDefnsXML_;
    return *this;
}

bool
MSLockMethod::operator == ( const MSLockMethod & rhs ) const
{
   if ( ( toleranceMethod_ == rhs.toleranceMethod_ ) &&
       ( adportable::is_equal( massToleranceDa_, rhs.massToleranceDa_ ) ) &&
       ( adportable::is_equal( massTolerancePpm_, rhs.massTolerancePpm_ ) ) &&
       ( refMassDefnsFullyQualifiedName_ == rhs.refMassDefnsFullyQualifiedName_ ) &&
       ( refMassDefnsXML_ == rhs.refMassDefnsXML_ ) ) {
           return true;
   }
   return false;
}

bool
MSLockMethod::operator != ( const MSLockMethod & rhs ) const
{
    return ! ( *this == rhs );
}

