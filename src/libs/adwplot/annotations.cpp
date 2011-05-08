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

#include "annotations.hpp"
#include "annotation.hpp"
#include "dataplot.hpp"

using namespace adwplot;

Annotations::Annotations( Dataplot& plot, vector_type& vec ) : plot_(plot), vec_(vec)
{
}

Annotations::Annotations( const Annotations& t ) : plot_( t.plot_ ), vec_(t.vec_)
{
}

void
Annotations::clear()
{
    return vec_.clear();
}


Annotation
Annotations::add( double x, double y, const std::wstring& title )
{
    vec_.push_back( Annotation( plot_, title, x, y ) );
    return vec_.back();
}