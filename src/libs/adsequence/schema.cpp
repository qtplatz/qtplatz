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

#include "schema.hpp"

using namespace adsequence;

schema::schema()
{
    schema_.push_back( column( "samp_type",      "Sample type",     COLUMN_SAMPLE_TYPE ) );
    schema_.push_back( column( "vial_num",       "Vial#",           COLUMN_INT ) );
    schema_.push_back( column( "samp_id",        "Sample Id",       COLUMN_INT ) ); // data file
    schema_.push_back( column( "injvol",         "Inj.(uL)",        COLUMN_DOUBLE ) );
    schema_.push_back( column( "run_length",     "Run length(min)", COLUMN_DOUBLE ) );
    schema_.push_back( column( "name_control",   "Control method",  COLUMN_VARCHAR ) );
    schema_.push_back( column( "name_process",   "Process method",  COLUMN_VARCHAR ) );
}

schema::schema( const schema& t ) : schema_( t.schema_ )
{
}

schema::vector_type::iterator
schema::begin()
{
    return schema_.begin();
}

schema::vector_type::iterator
schema::end()
{
    return schema_.end();
}

schema::vector_type::const_iterator
schema::begin() const
{
    return schema_.begin();
}

schema::vector_type::const_iterator
schema::end() const
{
    return schema_.end();
}

schema&
schema::operator << ( const column& c )
{
    schema_.push_back( c );
    return * this;
}

//////
column::column()
{
}

column::column( const column& t ) : name_( t.name_ )
                                  , display_name_( t.display_name_ )
                                  , type_( t.type_ )
{
}

column::column( const std::string& name
                , const std::string& display_name
                , COLUMN_TYPE x ) : name_( name )
                                  , display_name_( display_name )
                                  , type_( x )
{
}

const std::string&
column::name() const
{
    return name_;
}

const std::string&
column::display_name() const
{
    return display_name_;
}

COLUMN_TYPE
column::type() const
{
    return type_;
}
