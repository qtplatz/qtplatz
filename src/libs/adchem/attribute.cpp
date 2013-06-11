/**************************************************************************
** Copyright (C) 2013 MS-Cheminformatics LLC
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
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

#include "attribute.hpp"
#include <string>

using namespace adchem;

attribute::~attribute()
{
}

attribute::attribute()
{
}

attribute::attribute( const attribute& t ) : key_( t.key_ )
                                           , value_( t.value_ )
{
}

const char * 
attribute::key() const
{
    return key_.c_str();
}

const char * 
attribute::value() const
{
    return value_.c_str();
}

void 
attribute::key( const char * key )
{
    key_ = key;
}

void 
attribute::value( const char * value )
{
    value_ = value;
}

