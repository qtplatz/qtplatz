/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
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

#include "molecule.hpp"

using namespace adcontrols;
using namespace adcontrols::mol;

molecule::molecule()
{
}

molecule::molecule( const molecule& t ) : elements( t.elements )
                                        , cluster( t.cluster )
{
}

molecule&
molecule::operator << (const element& t)
{
    elements.push_back( t );
    return *this;
}

molecule& 
molecule::operator << (const isotope& t)
{
    cluster.push_back( t );
    return *this;
}

std::vector< element >::const_iterator 
molecule::element_begin() const
{
    return elements.begin();
}

std::vector< element >::const_iterator
molecule::element_end() const
{
    return elements.end();
}

