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
#include <QMetaEnum>
#include <QDebug>
#include <fstream>

// when load module by QLibrary()/dlopen() method, gcc built module does not expose RTTI information.
// here is the workaround

using namespace adplugin;

static const char * exclude_list [] = {
    "QWidgetResizeHandler"
    , "QAction"
    , "QWidget"
    , "QWidgetResizeHandler"
    , "QAction"
    , "QWidget"
    , "QPropertyAnimation"
    , "QPropertyAnimation"
    , "QWidgetResizeHandler"
    , "QAction"
    , "QWidget"
    , "QPropertyAnimation"
    , "QPropertyAnimation"
};

static bool is_exclude( const char * name )
{
    if ( *name == 'Q' ) {
        for ( std::size_t i = 0; i < sizeof( exclude_list ) / sizeof( exclude_list[0] ); ++i )
            if ( strcmp( name, exclude_list[ i ] ) == 0 )
                return true;
    }
    return false;
};

LifeCycleAccessor::LifeCycleAccessor( QObject * target ) :  pObject_( target )
                                                         , p_(0)
                                                         , conn_( false )
{
    p_ = dynamic_cast< adplugin::LifeCycle *>( pObject_ );
    if ( p_ == 0 ) {
        const char * name = target->metaObject()->className();
        if ( ! is_exclude( name ) ) {
            conn_ = connect( this, SIGNAL( trigger( adplugin::LifeCycle *& ) ), pObject_, SLOT( getLifeCycle( adplugin::LifeCycle *& ) ) );
#if defined DEBUG
            if ( ! conn_ ) {
                std::ofstream of( "exclude.txt", std::ios_base::app );
                of << "\"" << name << "\"" << std::endl;
            }
#endif
        }
    }
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
