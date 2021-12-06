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
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_serialize.hpp>
#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>

#include <array>
#include <adportable/float.hpp>

namespace boost {
    namespace serialization {

        using namespace adcontrols;

        template <class Archive >
        void serialize( Archive& ar, moltable::value_type& p, const unsigned int version ) {
            ar & BOOST_SERIALIZATION_NVP( p.enable_ );
            ar & BOOST_SERIALIZATION_NVP( p.flags_ );
            ar & BOOST_SERIALIZATION_NVP( p.mass_ );
            ar & BOOST_SERIALIZATION_NVP( p.abundance_ );
            ar & BOOST_SERIALIZATION_NVP( p.formula_ );
            ar & BOOST_SERIALIZATION_NVP( p.adducts_ );
            ar & BOOST_SERIALIZATION_NVP( p.synonym_ );
            ar & BOOST_SERIALIZATION_NVP( p.smiles_ );
            ar & BOOST_SERIALIZATION_NVP( p.description_ );
            if ( version >= 2 ) {
                ar & BOOST_SERIALIZATION_NVP( p.protocol_ );
                ar & BOOST_SERIALIZATION_NVP( p.properties_ );
            }
            if ( version >= 3 ) {
                ar & BOOST_SERIALIZATION_NVP( p.tR_ );
            }

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
            try {
                ar & BOOST_SERIALIZATION_NVP( data_ );
            } catch ( std::exception& ) {
                BOOST_THROW_EXCEPTION( serializer_error() << info( std::string( typeid(Archive).name() ) ) );
            }
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

BOOST_CLASS_VERSION( adcontrols::moltable::impl, 2 )

using namespace adcontrols;

bool
moltable::value_type::operator == ( const value_type& t ) const
{
#if 0
    ADDEBUG() << "== enable:" << std::make_pair( enable_, t.enable_ )
              << ", protocol:" << std::make_pair( protocol_, t.protocol_ )
              << ", formula:" << std::make_pair( formula_, t.formula_ )
              << ", adducts:" << std::make_pair( adducts_, t.adducts_ )
              << ", tR:" << std::make_pair( tR_, t.tR_ );
#endif
    return
        formula_ == t.formula_ &&
        adducts_ == t.adducts_ &&
        protocol_ == t.protocol_;
}

bool
moltable::value_type::isMSRef() const
{
    return ( flags_ & moltable::isMSRef ) == moltable::isMSRef;
}

void
moltable::value_type::setIsMSRef( bool on )
{
    flags_ = on ? moltable::isMSRef : 0;
}

boost::optional< int32_t >
moltable::value_type::protocol() const
{
    return protocol_;
}

void
moltable::value_type::setProtocol( boost::optional< int32_t >&& proto )
{
    protocol_ = proto;
}

boost::optional< double >
moltable::value_type::tR() const
{
    return tR_;
}

void
moltable::value_type::set_tR( boost::optional< double >&& tR )
{
    tR_ = tR;
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

    std::sort( impl_->data_.begin(), impl_->data_.end(), []( const value_type& a, const value_type& b ){ return a.mass() < b.mass(); } );

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
