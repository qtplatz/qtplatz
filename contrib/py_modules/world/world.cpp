// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2019 MS-Cheminformatics LLC
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

#include <iostream>
#include "world.hpp"

world::world()
{
    std::cout << "world ctor" << std::endl;
}

world::world( const world& t ) : msg_( t.msg_ )
{
    std::cout << "world copy" << std::endl;
}

world::~world()
{
    std::cout << "world dtor" << std::endl;
}

void
world::set( std::string msg )
{
    msg_ = msg;
}

double
world::mass() const
{
    return 9999.999;
}

std::string
world::greet()
{
    return msg_;
}

std::shared_ptr< world >
world::dup()
{
    return std::make_shared< world >( *this );
}
