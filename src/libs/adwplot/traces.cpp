// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "traces.hpp"
#include "trace.hpp"
#include "dataplot.hpp"
#include <qtwrapper/qstring.hpp>

using namespace adwplot;

Traces::Traces( Dataplot& plot, vector_type& vec ) : plot_( plot ), vec_(vec)
{
}

Traces::Traces( const Traces& t ) : plot_( t.plot_ ), vec_(t.vec_)
{
}

size_t
Traces::size() const
{
    return vec_.size();
}

void
Traces::clear()
{
    return vec_.clear();
}

Trace
Traces::add( const std::wstring& title )
{
    vec_.push_back( Trace( plot_, title ) );
    return vec_.back();
}

