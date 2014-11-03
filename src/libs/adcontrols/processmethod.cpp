/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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
#include <compiler/workaround.h>
#include <compiler/disable_unused_parameter.h>
#if defined _MSC_VER
#pragma warning(disable:4996)
#endif

#include "processmethod.hpp"
#include "centroidmethod.hpp"
#include "isotopemethod.hpp"
#include "elementalcompositionmethod.hpp"
#include "mscalibratemethod.hpp"
#include "msreferences.hpp"
#include "msreference.hpp"
#include "peakmethod.hpp"
#include "serializer.hpp"
#include "targetingmethod.hpp"

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

#include <compiler/diagnostic_push.h>
#include <compiler/disable_unused_local_typedefs.h>

#include <boost/serialization/variant.hpp>
#include <compiler/diagnostic_pop.h>

#include <boost/serialization/base_object.hpp>
#include <boost/uuid/uuid_serialize.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#if defined _MSC_VER
#pragma warning(default:4996)
#endif

using namespace adcontrols;

ProcessMethod::~ProcessMethod()
{
}

ProcessMethod::ProcessMethod()
{
}

ProcessMethod::ProcessMethod( const ProcessMethod& t ) : vec_( t.vec_ )
{
}

//////
template<class T> struct method_finder {
    static const T * find( const ProcessMethod::vector_type& vec ) {
		auto it = std::find_if( vec.begin(), vec.end(), [&]( const ProcessMethod::value_type& t ){
                return typeid(T) == t.type();
            });
		if ( it != vec.end() )
			return &boost::get< T >( *it );
        return 0;
    }
};

ProcessMethod&
ProcessMethod::operator *= ( const ProcessMethod& t )
{
    for ( auto& rhs: t ) {
        auto it = std::remove_if( vec_.begin(), vec_.end(), [=] ( const value_type& lhs ){ return lhs.type() == rhs.type(); } );
        if ( it != vec_.end() )
            vec_.erase( it, vec_.end() );
    }

    for ( auto& rhs: t )
        vec_.push_back( rhs );
    
    return *this;
}

///////////

const ProcessMethod::value_type&
ProcessMethod::operator [] ( int idx ) const
{
    return vec_[idx];
}

ProcessMethod::value_type&
ProcessMethod::operator [] ( int idx )
{
    return vec_[idx];
}

void
ProcessMethod::clear()
{
    vec_.clear();
}

void
ProcessMethod::erase( iterator beg, iterator end )
{
    vec_.erase( beg, end );
}

size_t
ProcessMethod::size() const
{
    return vec_.size();
}

ProcessMethod::vector_type::iterator
ProcessMethod::begin()
{
    return vec_.begin();
}

ProcessMethod::vector_type::iterator
ProcessMethod::end()
{
    return vec_.end();
}

ProcessMethod::vector_type::const_iterator
ProcessMethod::begin() const
{
    return vec_.begin();
}

ProcessMethod::vector_type::const_iterator
ProcessMethod::end() const
{
    return vec_.end();
}

//////////////////// serialize /////////////////

bool
ProcessMethod::archive( std::ostream& os, const ProcessMethod& t )
{
    portable_binary_oarchive ar( os );
    ar << t;
    return true;
}

bool
ProcessMethod::restore( std::istream& is, ProcessMethod& t )
{
    portable_binary_iarchive ar( is );
    ar >> t;
    return true;
}

//static
bool
ProcessMethod::xml_archive( std::wostream& os, const ProcessMethod& t )
{
    return internal::xmlSerializer("ProcessMethod").archive( os, t );
}

//static
bool
ProcessMethod::xml_restore( std::wistream& is, ProcessMethod& t )
{
    return internal::xmlSerializer("ProcessMethod").restore( is, t );
}
