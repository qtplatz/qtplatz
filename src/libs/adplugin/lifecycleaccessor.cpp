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

#include "lifecycleaccessor.hpp"
#include "lifecycle.hpp"

// when load module by QLibrary()/dlopen() method, gcc built module does not expose RTTI information.
// here is the workaround

using namespace adplugin;

LifeCycleAccessor::LifeCycleAccessor( QObject * target ) :  pObject_( target )
                                                         , p_(0)
                                                         , conn_( false )
{
    p_ = dynamic_cast< adplugin::LifeCycle *>( pObject_ );
    if ( p_ == 0 )
        conn_ = connect( this, SIGNAL( trigger( adplugin::LifeCycle *& ) ), pObject_, SLOT( getLifeCycle( adplugin::LifeCycle *& ) ) );
}

LifeCycleAccessor::~LifeCycleAccessor()
{
    if ( conn_ )
        disconnect( this, SIGNAL( trigger( adplugin::LifeCycle *& ) ), pObject_, SLOT( getLifeCycle( adplugin::LifeCycle *& ) ) );
}

adplugin::LifeCycle *
LifeCycleAccessor::get()
{
    if ( p_ )
        return p_;
    else
        emit trigger( p_ );
    return p_;
}
