/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
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

#include "countingmethod.hpp"
#include "constants.hpp"
#include "serializer.hpp"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/archive/archive_exception.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace boost {
    namespace serialization {

        template< class Archive >
        void serialize( Archive& ar, adcontrols::CountingMethod::value_type& _, const unsigned int version )
        {
            using adcontrols::CountingMethod;
            ar & boost::serialization::make_nvp( "enable",   get<CountingMethod::CountingEnable>( _ ) );
            ar & boost::serialization::make_nvp( "formula",  get<CountingMethod::CountingFormula>( _ ) );
            ar & boost::serialization::make_nvp( "range",    get<CountingMethod::CountingRange>( _ ) );
            ar & boost::serialization::make_nvp( "protocol", get<CountingMethod::CountingProtocol>( _ ) );
        }
        
    }
}


namespace adcontrols {

    template< typename T = CountingMethod >
    class CountingMethod_archive {
    public:
        template<class Archive>
        void serialize( Archive& ar, T& _, const unsigned int version ) {
            ar & BOOST_SERIALIZATION_NVP( _.enable_ );
            ar & BOOST_SERIALIZATION_NVP( _.values_ );
        }
    };
    
    ///////// Portable binary archive ////////    
    template<> ADCONTROLSSHARED_EXPORT void
    CountingMethod::serialize( portable_binary_oarchive& ar, const unsigned int version ) {
        CountingMethod_archive<>().serialize( ar, *this, version );
    }

    template<> ADCONTROLSSHARED_EXPORT void
    CountingMethod::serialize( portable_binary_iarchive& ar, const unsigned int version ) {
        CountingMethod_archive<>().serialize( ar, *this, version );
    }

    ///////// XML archive ////////
    template<> void
    CountingMethod::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        CountingMethod_archive<>().serialize( ar, *this, version );
    }

    template<> void
    CountingMethod::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        CountingMethod_archive<>().serialize( ar, *this, version );        
    }

}

using namespace adcontrols;

CountingMethod::CountingMethod() : enable_( true )
{
}

CountingMethod::CountingMethod( const CountingMethod& t ) : values_( t.values_ )
{
}

CountingMethod&
CountingMethod::operator = ( const CountingMethod& rhs )
{
    values_ = rhs.values_;
	return *this;
}

size_t
CountingMethod::size() const
{
    return values_.size();
}

void
CountingMethod::clear()
{
    values_.clear();
}

CountingMethod::value_type&
CountingMethod::operator [] ( size_t idx )
{
    return values_[ idx ];
}

const CountingMethod::value_type&
CountingMethod::operator [] ( size_t idx ) const
{
    return values_[ idx ];
}

CountingMethod::iterator
CountingMethod::begin()
{
    return values_.begin();
}

CountingMethod::iterator
CountingMethod::end()
{
    return values_.end();
}

CountingMethod::const_iterator
CountingMethod::begin() const
{
    return values_.begin();
}

CountingMethod::const_iterator
CountingMethod::end() const
{
    return values_.end();
}

//static
const boost::uuids::uuid&
CountingMethod::clsid()
{
    static const boost::uuids::uuid myclsid = boost::uuids::name_generator( adcontrols_uuid )( "adcontrols::CountingMethod" );
    return myclsid;
}

// static
const char *
CountingMethod::modelClass()
{
    return "Counting";
}

// static
const char *
CountingMethod::itemLabel()
{
    return "Counting";
}

CountingMethod&
CountingMethod::operator << ( value_type&& t )
{
    values_.emplace_back( t );
    return * this;
}

bool
CountingMethod::enable() const
{
    return enable_;
}

void
CountingMethod::setEnable( bool b )
{
    enable_ = b;
}

bool
CountingMethod::archive( std::ostream& os, const CountingMethod& t )
{
    try {
        portable_binary_oarchive ar( os );
        ar & t;
        return true;
    } catch ( std::exception& ex ) {
        BOOST_THROW_EXCEPTION( ex );
    }
    return false;
}
    
bool
CountingMethod::restore( std::istream& is, CountingMethod& t )
{
    try {
        portable_binary_iarchive ar( is );
        ar & t;
        return true;
    } catch ( std::exception& ex ) {
        BOOST_THROW_EXCEPTION( ex );
    }
    return false;
}
    
bool
CountingMethod::xml_archive( std::wostream& os, const CountingMethod& t )
{
    try {
        boost::archive::xml_woarchive ar( os );
        ar & boost::serialization::make_nvp( "m", t );
        return true;
    } catch ( std::exception& ex ) {
        BOOST_THROW_EXCEPTION( ex );
    }
    return false;
}
    
bool
CountingMethod::xml_restore( std::wistream& is, CountingMethod& t )
{
    try {
        boost::archive::xml_wiarchive ar( is );
        ar & boost::serialization::make_nvp( "m", t );
        return true;
    } catch ( std::exception& ex ) {
        BOOST_THROW_EXCEPTION( ex );
    }
    return false;
        
}
