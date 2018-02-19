// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include "baselines.hpp"
#include "baseline.hpp"

using namespace adcontrols;

Baselines::~Baselines()
{
}

Baselines::Baselines() : nextId_(1)
{
}

Baselines::Baselines( const Baselines& t ) : nextId_( t.nextId_ )
                                           , baselines_( t.baselines_ ) 
{
}

Baselines&
Baselines::operator = ( const Baselines& t )
{
    nextId_ = t.nextId_;
    baselines_ = t.baselines_;
    return *this;
}

int
Baselines::add( const Baseline& t )
{
    baselines_.push_back( t );

	Baseline& bs = baselines_.back();
	bs.setBaseId( nextId_++ );
	return bs.baseId();
}

int
Baselines::nextId( bool increment )
{
    if ( increment )
        ++nextId_;
    return nextId_;
}



