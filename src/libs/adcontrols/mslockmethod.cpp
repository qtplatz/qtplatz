//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "mslockmethod.h"
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

