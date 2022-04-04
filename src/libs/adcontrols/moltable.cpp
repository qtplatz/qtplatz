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
#include "constants.hpp"
#include <adportable/float.hpp>
#include <adportable/json_helper.hpp>
#include <adportable/json/extract.hpp>
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

        // compatibility only
        typedef boost::variant< bool, uint32_t, double, std::string, boost::uuids::uuid > custom_type;

        template <class Archive >
        void serialize( Archive& ar, moltable::value_type& p, const unsigned int version ) {
            if ( version >= 4 ) {
                try {
                    ar & BOOST_SERIALIZATION_NVP( p.enable_ );
                    ar & BOOST_SERIALIZATION_NVP( p.flags_ );
                    ar & BOOST_SERIALIZATION_NVP( p.mass_ );
                    ar & BOOST_SERIALIZATION_NVP( p.abundance_ );
                    ar & BOOST_SERIALIZATION_NVP( p.formula_ );
                    ar & boost::serialization::make_nvp("adduct_pos", std::get< 0 >( p.adducts_ ) );
                    ar & boost::serialization::make_nvp("adduct_neg", std::get< 1 >( p.adducts_ ) );
                    ar & BOOST_SERIALIZATION_NVP( p.synonym_ );
                    ar & BOOST_SERIALIZATION_NVP( p.smiles_ );
                    ar & BOOST_SERIALIZATION_NVP( p.description_ );
                    ar & BOOST_SERIALIZATION_NVP( p.protocol_ );
                    ar & BOOST_SERIALIZATION_NVP( p.tR_ );
                    ar & BOOST_SERIALIZATION_NVP( p.molid_ );
                    ar & BOOST_SERIALIZATION_NVP( p.resv_ );
                } catch ( std::exception& ex ) {
                    BOOST_THROW_EXCEPTION( std::runtime_error( ex.what() ) );
                }
            } else {
                std::vector < std::pair< std::string, custom_type > > properties;
                ar & BOOST_SERIALIZATION_NVP( p.enable_ );
                ar & BOOST_SERIALIZATION_NVP( p.flags_ );
                ar & BOOST_SERIALIZATION_NVP( p.mass_ );
                ar & BOOST_SERIALIZATION_NVP( p.abundance_ );
                ar & BOOST_SERIALIZATION_NVP( p.formula_ );
                ar & BOOST_SERIALIZATION_NVP( std::get< 0 >( p.adducts_ ) );
                ar & BOOST_SERIALIZATION_NVP( p.synonym_ );
                ar & BOOST_SERIALIZATION_NVP( p.smiles_ );
                ar & BOOST_SERIALIZATION_NVP( p.description_ );
                if ( version >= 2 ) {
                    ar & BOOST_SERIALIZATION_NVP( p.protocol_ );
                    ar & BOOST_SERIALIZATION_NVP( properties );
                }
                if ( version >= 3 ) {
                    ar & BOOST_SERIALIZATION_NVP( p.tR_ );
                }
                auto it = std::find_if( properties.begin(), properties.end(), []( const auto& t ){ return t.first == "molid"; } );
                if ( it != properties.end() ) {
                    p.molid_ = boost::get< boost::uuids::uuid >( it->second );
                }
            }
        }
    }
}

namespace adcontrols {

    class moltable::impl {
    public:
        std::vector< value_type > data_;
        ion_polarity polarity_;

        friend class boost::serialization::access;
        template<class Archive>
        void serialize( Archive& ar, const unsigned int version ) {
            using namespace boost::serialization;
            try {
                if ( version >= 5 ) {
                    ar & BOOST_SERIALIZATION_NVP( data_ );
                    ar & BOOST_SERIALIZATION_NVP( polarity_ );
                } else {
                    ar & BOOST_SERIALIZATION_NVP( data_ );
                }
            } catch ( std::exception& ) {
                BOOST_THROW_EXCEPTION( serializer_error() << info( std::string( typeid(Archive).name() ) ) );
            }
        }

        impl() : polarity_( adcontrols::polarity_positive ) {
        }

        impl( const impl& t ) : data_( t.data_ )
                              , polarity_( t.polarity_ ) {
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

BOOST_CLASS_VERSION( adcontrols::moltable::impl, 5 )

using namespace adcontrols;

moltable::value_type::value_type()
    : enable_( true ), flags_( 0 ), mass_( 0 ), abundance_( 1.0 )
    , protocol_( boost::none ), tR_( boost::none )
    , resv_( 0 )
{
}

moltable::value_type::value_type( const value_type& t )
    : enable_( t.enable_ )
    , flags_( t.flags_ )
    , mass_( t.mass_ )
    , abundance_( t.abundance_ )
    , formula_( t.formula_ )
    , adducts_( t.adducts_ )
    , synonym_( t.synonym_ )
    , smiles_( t.smiles_ )
    , description_( t.description_ )
    , protocol_( t.protocol_ )
    , tR_( t.tR_ )
    , resv_( t.resv_ )
{
}

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

std::string&
moltable::value_type::adducts()
{
    return std::get< polarity_positive >( adducts_ );
    // return polarity_ == polarity_positive
    //     ? std::get< polarity_positive >( adducts_ )
    //     : std::get< polarity_negative >( adducts_ );
}

const std::string&
moltable::value_type::adducts() const
{
    return std::get< polarity_positive >( adducts_ );
    // return polarity_ == polarity_positive
    //     ? std::get< polarity_positive >( adducts_ )
    //     : std::get< polarity_negative >( adducts_ );
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

boost::optional< boost::uuids::uuid >
moltable::value_type::molid() const
{
    return molid_;
}

void
moltable::value_type::setMolid( boost::optional< boost::uuids::uuid >&& uuid )
{
    molid_ = uuid;
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

ion_polarity&
moltable::polarity()
{
    return impl_->polarity_;
}

const ion_polarity&
moltable::polarity() const
{
    return impl_->polarity_;
}

void
moltable::setPolarity( ion_polarity t )
{
    impl_->polarity_ = t;
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

namespace adcontrols {

    void tag_invoke( boost::json::value_from_tag, boost::json::value& jv, const moltable::value_type& t )
    {
        jv = boost::json::object{
            { "enable",        t.enable_     }
            , { "flags",       t.flags_      }
            , { "mass",        t.mass_       }
            , { "abundance",   t.abundance_  }
            , { "formula",     t.formula_    }
            , { "adducts",     t.adducts_    }
            , { "synonym",     t.synonym_    }
            , { "smiles",      t.smiles_     }
            , { "description", t.description_}
        };
        auto obj = jv.as_object();
        if ( t.protocol_ ) { obj[ "protocol" ]   = *t.protocol_; }
        if ( t.tR_ )       { obj[ "tR" ]         = *t.tR_;       }
        if ( t.molid_ )    { obj[ "molid"    ]   = boost::lexical_cast< std::string >( *t.molid_ ); }
    }

    moltable::value_type tag_invoke( boost::json::value_to_tag< moltable::value_type >&, const boost::json::value& jv )
    {
        moltable::value_type t;
        if ( jv.is_object() ) {
            using namespace adportable::json;
            auto obj = jv.as_object();
            extract( obj, t.enable_,      "enable"      );
            extract( obj, t.flags_,       "flags"       );
            extract( obj, t.mass_,        "mass"        );
            if ( auto adducts = obj.if_contains( "adducts" ) ) {
                if ( adducts->is_array() ) { // v4 data
                    t.adducts_ = boost::json::value_to< decltype( t.adducts_ ) >( *adducts );
                } else {
                    std::get< 0 >( t.adducts_ ) = boost::json::value_to< std::string >( *adducts );
                }
            }
            extract( obj, t.abundance_,   "abundance"   );
            extract( obj, t.formula_,     "formula"     );
            extract( obj, t.adducts_,     "adducts"     ); // check if array then v4 else v3 data
            extract( obj, t.synonym_,     "synonym"     );
            extract( obj, t.smiles_,      "smiles"      );
            extract( obj, t.description_, "description" );

            //
            if ( auto protocol = obj.if_contains( "protocol" ) ) {
                t.protocol_ = boost::json::value_to< int32_t >( *protocol );
            }
            if ( auto tR = obj.if_contains( "tR" ) ) {
                t.tR_ = boost::json::value_to< double >( *tR );
            }
            if ( auto molid = obj.if_contains( "molid" ) ) {
                t.molid_ = boost::lexical_cast< boost::uuids::uuid >( boost::json::value_to< std::string >( *molid ) );
            }
        }
        return t;
    }

    ////////
    void
    tag_invoke( boost::json::value_from_tag, boost::json::value& jv, const moltable& t )
    {
        jv = boost::json::object{{ "moltable"
                , {
                    {   "data",      t.data() }
                    , { "polarity",   static_cast< uint32_t >( t.polarity() ) }
                }
            }};
    }

    moltable
    tag_invoke( boost::json::value_to_tag< moltable >&, const boost::json::value& jv )
    {
        moltable t;

        using namespace adportable::json;
        if ( jv.is_object() ) {
            if ( const auto&  moltable = jv.as_object().if_contains( "moltable" ) ) {
                if ( moltable->is_object() ) {
                    auto& obj = moltable->as_object();
                    extract( obj, t.data(), "data" );
                    if ( obj.if_contains( "polarity" ) ) {
                        extract( obj, reinterpret_cast< uint32_t& >(t.polarity()), "polarity" );
                    }
                }
            }
        }
        return t;
    }
}
