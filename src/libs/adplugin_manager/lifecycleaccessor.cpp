/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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
#include <adplugin_manager/lifecycle.hpp>
#include <QDebug>
#include <QMetaEnum>
#include <fstream>

// when load module by QLibrary()/dlopen() method, gcc built module does not expose RTTI information.
// here is the workaround

using namespace adplugin;

LifeCycleAccessor::LifeCycleAccessor( QObject * target ) : pObject_( target )
                                                         , p_(0)
                                                         , conn_( false )
{
    p_ = qobject_cast< adplugin::LifeCycle *>( pObject_ );

    

    if ( p_ == 0 ) {
        if ( ( p_ = dynamic_cast<adplugin::LifeCycle *>( pObject_ ) ) ) {

            const char * name = target->metaObject()->className();

            Q_ASSERT( p_ ); // if this is true, target is inherit from LifeCycle but not Q_INTERFACES decleard.
                            // dynamic_cast may not work for dynamically loaded objects on Linux and Mac 
                            // but on Windows with RTTI enabled.
        }

        if ( p_ == 0 ) {
            conn_ = connect( this, SIGNAL( trigger( adplugin::LifeCycle *& ) ), pObject_, SLOT( getLifeCycle( adplugin::LifeCycle *& ) ) );
            emit trigger( p_ );
            disconnect( this, SIGNAL( trigger( adplugin::LifeCycle *& ) ), pObject_, SLOT( getLifeCycle( adplugin::LifeCycle *& ) ) );
        }
    }
}

LifeCycleAccessor::~LifeCycleAccessor()
{
}

adplugin::LifeCycle *
LifeCycleAccessor::get()
{
    return p_;
}
