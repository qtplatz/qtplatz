/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
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

#include "methodeditorbehavior.hpp"

using namespace infitofcontrols;

MethodEditorBehavior::MethodEditorBehavior() : gate1_( std::make_tuple( true, 0.000e-6, 0.010e-6, false ) ) // helium counting defalut
                                             , gate2_( std::make_tuple( true, 6.320e-6, 0.100e-6, true ) )
                                             , inject_( std::make_tuple( true, -1.0e-6, 1.32e-6 ) )
{
}

MethodEditorBehavior::MethodEditorBehavior( const MethodEditorBehavior& t ) : gate1_( t.gate1_ )
                                                                            , gate2_( t.gate2_ )
                                                                            , inject_( t.inject_ )
{
}

bool
MethodEditorBehavior::gateEditable( int id ) const
{
    switch( id ) {
    case 0:
        return std::get< 0 >( gate1_ );
        break;
    case 1:
        return std::get< 0 >( gate2_ );
        break;
    }
    return false;
}

bool
MethodEditorBehavior::injectEditable() const
{
    return std::get< 0 >( inject_ );
}

void
MethodEditorBehavior::setGateEditable( int id, bool e )
{
    switch( id ) {
    case 0:
        std::get< 0 >( gate1_ ) = e;
        break;
    case 1:
        std::get< 0 >( gate2_ ) = e;
        break;
    }
}

void
MethodEditorBehavior::setGateEnable( int id, bool e )
{
    switch( id ) {
    case 0:
        std::get< 3 >( gate1_ ) = e;
        break;
    case 1:
        std::get< 3 >( gate2_ ) = e;
        break;
    }
}

void
MethodEditorBehavior::setInjectEditable( bool e )
{
    std::get< 0 >( inject_ ) = e;
}

bool
MethodEditorBehavior::gateEnable( int id ) const
{
    switch( id ) {
    case 0:
        return std::get< 3 >( gate1_ );
        break;
    case 1:
        return std::get< 3 >( gate2_ );
        break;
    }
    return false;
}

std::pair< double, double >
MethodEditorBehavior::gate( int id ) const
{
    switch( id ) {
    case 0:
        return std::make_pair( std::get< 1 >( gate1_ ), std::get< 2 >( gate1_ ) );
        break;
    case 1:
        return std::make_pair( std::get< 1 >( gate2_ ), std::get< 2 >( gate2_ ) );
        break;
    }
    return std::make_pair( 0, 0 );
}

std::pair< double, double >
MethodEditorBehavior::inject() const
{
    return std::make_pair( std::get< 1 >( inject_ ), std::get< 2 >( inject_ ) );
}

void
MethodEditorBehavior::setGate( int id, double delay, double width )
{
    switch( id ) {
    case 0:
        std::get< 1 >( gate1_ ) = delay;
        std::get< 2 >( gate1_ ) = width;
        break;
    case 1:
        std::get< 1 >( gate2_ ) = delay;
        std::get< 2 >( gate2_ ) = width;
        break;
    }
}

void
MethodEditorBehavior::setInject( double delay, double width )
{
    std::get< 1 >( inject_ ) = delay;
    std::get< 2 >( inject_ ) = width;
}
