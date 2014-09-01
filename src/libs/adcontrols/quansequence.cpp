/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include "quansequence.hpp"
#include "serializer.hpp"
#include <adportable/uuid.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/uuid/uuid_serialize.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>

using namespace adcontrols;

QuanSequence::~QuanSequence()
{
}

QuanSequence::QuanSequence()
{
}

QuanSequence::QuanSequence( const QuanSequence& t ) : ident_( t.ident_ ), samples_( t.samples_ )
{
}

QuanSequence&
QuanSequence::operator << ( const QuanSample& t )
{
    int32_t rowid = int32_t( samples_.size() );
    samples_.push_back( t );
    samples_.back().sequence_uuid( ident_.uuid(), rowid );
    return *this;
}

//static
bool
QuanSequence::archive( std::ostream& os, const QuanSequence& t )
{
    portable_binary_oarchive ar( os );
    ar << t;
    return true;
}

//static
bool
QuanSequence::restore( std::istream& is, QuanSequence& t )
{
    portable_binary_iarchive ar( is );
    ar >> t;
    return true;
}

//static
bool
QuanSequence::xml_archive( std::wostream& os, const QuanSequence& t )
{
    return internal::xmlSerializer("QuanSequence").archive( os, t );
}

//static
bool
QuanSequence::xml_restore( std::wistream& is, QuanSequence& t )
{
    return internal::xmlSerializer("QuanSequence").restore( is, t );
}
