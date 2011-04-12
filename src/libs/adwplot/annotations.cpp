// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#include "annotations.h"
#include "annotation.h"
#include "dataplot.h"

using namespace adwplot;

Annotations::Annotations( Dataplot& plot ) : plot_(plot)
{
}

Annotations::Annotations( const Annotations& t ) : plot_( t.plot_ )
{
}

size_t
Annotations::size() const
{
    return plot_.get< vector_type& >().size();
}

void
Annotations::clear()
{
    return plot_.get< vector_type& >().clear();
}


Annotations::vector_type::iterator 
Annotations::begin()
{
    return plot_.get< vector_type& >().begin();
}

Annotations::vector_type::iterator
Annotations::end()
{
    return plot_.get< vector_type& >().end();
}

Annotations::vector_type::const_iterator
Annotations::begin() const
{
    return plot_.get< vector_type& >().begin();
}

Annotations::vector_type::const_iterator
Annotations::end() const
{
    return plot_.get< vector_type& >().end();
}

Annotation
Annotations::add( double x, double y, const std::wstring& title )
{
    vector_type& annos = plot_.get< vector_type &>();
    annos.push_back( Annotation( plot_, title, x, y ) );
    return annos.back();
}