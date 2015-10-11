/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
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

#pragma once

#include "error_code.hpp"

using namespace adcontrols;

error_code::error_code() : code_( 0 ), cat_( noerror )
{
}

error_code::error_code( const error_code& t ) : ec_( t.ec_ )
                                              , cat_( t.cat_ )
                                              , message_( t.message_ )
{
}


void
error_code::assign( int code, const std::string& message )
{
    code_ = code;
    message_ = message;
}

void
error_code::assign( std::error_code& ec )
{
    ec_ = ec;
}


std::string
error_code::message() const
{
    if ( ec_ )
        return ec_.message();
    return message_;
}

