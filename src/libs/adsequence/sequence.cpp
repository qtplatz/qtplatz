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
#include <boost/foreach.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>

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

line_t&
sequence::operator [] ( size_t row )
{
    if ( lines_.size() > row )
        return lines_[ row ];
    throw std::exception();
}

const line_t&
sequence::operator [] ( size_t row ) const
{
    if ( lines_.size() > row )
        return lines_[ row ];
    throw std::exception();
}

void
sequence::operator << ( const line_t& t )
{
    lines_.push_back( t );
}

adsequence::line_t
sequence::make_line() const
{
    const adsequence::schema& schema = *schema_;

    adsequence::line_t line;
    for ( schema::vector_type::const_iterator
              it = schema.begin(); it != schema.end(); ++it ) {
        switch ( it->type() ) {
        case COLUMN_INT:  case COLUMN_SAMPLE_TYPE:
			line.push_back( int(0) );
            break;
        case COLUMN_DOUBLE:
            line.push_back( double(0) );
            break;
        case COLUMN_VARCHAR:
            line.push_back( std::wstring(L"") );
            break;
        }
    }
    return line;
}

size_t
sequence::size() const
{
    return lines_.size();
}

void
sequence::clear()
{
    lines_.clear();
}

sequence::method_vector_type&
sequence::getControlMethod()
{
    return control_methods_;
}

const sequence::method_vector_type&
sequence::getControlMethod() const
{
    return control_methods_;
}

sequence::method_vector_type&
sequence::getProcessMethod()
{
    return process_methods_;
}

const sequence::method_vector_type&
sequence::getProcessMethod() const
{
    return process_methods_;
}

//static
bool
sequence::xml_archive( std::ostream& out, const sequence& sequence )
{
	try {
		boost::archive::xml_oarchive ar( out );
		ar << BOOST_SERIALIZATION_NVP( sequence );
	} catch ( std::exception& ) {
		return false;
	}
	return true;
}

//static
bool
sequence::xml_restore( std::istream& in, sequence& sequence )
{
	try {
		boost::archive::xml_iarchive ar( in );
		ar >> BOOST_SERIALIZATION_NVP( sequence );
	} catch ( std::exception& ) {
		return false;
	}
	return true;
}

//static
bool
sequence::archive( std::ostream& out, const sequence& sequence )
{
	try {
		portable_binary_oarchive ar( out );
		ar << sequence;
	} catch ( std::exception& ) {
		return false;
	}
	return true;
}

//static
bool
sequence::restore( std::istream& in, sequence& sequence )
{
	try {
		portable_binary_iarchive ar( in );
		ar >> sequence;
	} catch ( std::exception& ) {
		return false;
	}
	return true;
}

