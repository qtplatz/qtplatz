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

#include "peptides.hpp"

using namespace adprot;

peptides::peptides()
{
}

peptides::size_type
peptides::size() const
{
    return vec_.size();
}

peptides::iterator
peptides::begin()
{
    return vec_.begin();
}

peptides::iterator
peptides::end()
{
    return vec_.end();
}

peptides::const_iterator
peptides::begin() const
{
    return vec_.begin();
}

peptides::const_iterator
peptides::end() const
{
    return vec_.end();
}

peptides::iterator
peptides::erase( iterator first, iterator last )
{
    return vec_.erase( first, last );
}

peptides&
peptides::operator << ( const peptide& p )
{
    vec_.push_back( p );
    return *this;
}
