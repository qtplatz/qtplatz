/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "moltable.hpp"
#include "serializer.hpp"
#include <adportable/float.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/utility.hpp>

#include <array>
#include <adportable/float.hpp>

namespace boost {
    namespace serialization {

        using namespace adcontrols;

        template <class Archive >
        void serialize( Archive& ar, moltable::value_type& p, const unsigned int ) {
            ar & BOOST_SERIALIZATION_NVP( p.enable() );
            ar & BOOST_SERIALIZATION_NVP( p.flags() );
            ar & BOOST_SERIALIZATION_NVP( p.mass() );
            ar & BOOST_SERIALIZATION_NVP( p.abundance() );
            ar & BOOST_SERIALIZATION_NVP( p.formula() );
            ar & BOOST_SERIALIZATION_NVP( p.adducts() );
            ar & BOOST_SERIALIZATION_NVP( p.synonym() );
            ar & BOOST_SERIALIZATION_NVP( p.smiles() );
            ar & BOOST_SERIALIZATION_NVP( p.description() );
        }
    }
}

namespace adcontrols {

    class moltable::impl {
    public:
        std::vector< value_type > data_;
        
        friend class boost::serialization::access;
        template<class Archive>
        void serialize( Archive& ar, const unsigned int version ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( data_ );
        }
        
        impl() {
        }

        impl( const impl& t ) : data_( t.data_ ) {
        }
    };

    ////////// PORTABLE BINARY ARCHIVE //////////
    template<> void
    moltable::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        ar & *impl_;
    }

    template<> void
    moltable::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        ar & *impl_;
    }

    ///////// XML archive ////////
    template<> void
    moltable::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        ar & boost::serialization::make_nvp( "impl", *impl_ );
    }

    template<> void
    moltable::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        ar & boost::serialization::make_nvp( "impl", *impl_ );
    }
}

BOOST_CLASS_VERSION( adcontrols::moltable::impl, 1 )

using namespace adcontrols;

bool
moltable::value_type::isMSRef() const
{
    return ( flags_ & moltable::isMSRef ) == moltable::isMSRef;
}

void
moltable::value_type::setIsMSRef( bool on )
{
    //flags = on ? flags & moltable::isMSRef : flags & ~moltable::isMSRef;
    flags_ = on ? moltable::isMSRef : 0;
}


moltable::~moltable()
{
    delete impl_;
}

moltable::moltable() : impl_( new impl() )
{
}

moltable::moltable( const moltable& t ) : impl_( new impl( *t.impl_ ) )
{
}

moltable&
moltable::operator = ( const moltable& t )
{
    impl_->data_ = t.impl_->data_;
    return *this;
}

moltable&
moltable::operator += ( const moltable& t )
{
    std::move( t.impl_->data_.begin(), t.impl_->data_.end(), std::back_inserter( impl_->data_ ) );

    std::sort( impl_->data_.begin(), impl_->data_.end(), []( value_type& a, value_type& b ){ return a.mass() < b.mass(); } );

    auto it = std::unique( impl_->data_.begin(), impl_->data_.end()
                         , []( value_type& a, value_type& b ){ return adportable::compare<double>::essentiallyEqual(a.mass(), b.mass()); } );

    impl_->data_.erase( it, impl_->data_.end() );

    return *this;
}

const std::vector< moltable::value_type >&
moltable::data() const
{
    return impl_->data_;
}

std::vector< moltable::value_type >&
moltable::data()
{
    return impl_->data_;
}

moltable&
moltable::operator << ( const value_type& v )
{
    impl_->data_.push_back( v );
    return *this;
}

bool
moltable::empty() const
{
    return impl_->data_.empty();
}

size_t
moltable::size() const
{
    return impl_->data_.size();
}

//static
bool
moltable::xml_archive( std::wostream& os, const moltable& t )
{
    return internal::xmlSerializer("moltable").archive( os, t );
}

//static
bool
moltable::xml_restore( std::wistream& is, moltable& t )
{
    return internal::xmlSerializer("moltable").restore( is, t );
}
