/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include "sequence.hpp"
#include "schema.hpp"

using namespace adsequence;

sequence::~sequence()
{
    delete schema_;
}

sequence::sequence() : schema_( new adsequence::schema )
{
}

sequence::sequence( const sequence& t ) : schema_( new adsequence::schema( *t.schema_ ) )
                                        , lines_( t.lines_ )
                                        , control_methods_( t.control_methods_ )
                                        , process_methods_( t.process_methods_ )
{
}

const adsequence::schema&
sequence::schema() const
{
    return *schema_;
}

void
sequence::schema( const adsequence::schema& t )
{
    delete schema_;
    schema_ = new adsequence::schema( t );
    lines_.clear();
}
