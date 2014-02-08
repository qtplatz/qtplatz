/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "protein.hpp"

using namespace adpeptide;

protein::protein()
{
}

protein::protein( const protein& t ) : name_( t.name_ )
                                     , sequence_( t.sequence_ )
{
}

protein::protein( const std::string& name
                  , const std::string& sequence ) : name_( name )
                                                  , sequence_( sequence )
{
}

const std::string&
protein::name() const
{
    return name_;
}

void
protein::name( const std::string& var )
{
    name_ = var;
}


const std::string&
protein::sequence() const
{
    return sequence_;
}

void
protein::sequence( const std::string& var )
{
    sequence_ = var;
}

