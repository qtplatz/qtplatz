// This is a -*- C++ -*- header.
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

#include "controlmethod.hpp"
#include "serializer.hpp"
#include "idaudit.hpp"
#include <adportable/float.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <workaround/boost/uuid/uuid_serialize.hpp>
#include <adportable/base64.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/any.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_serialize.hpp>

namespace adcontrols {

    namespace ControlMethod {

        class Method::impl {
        public:
            impl() {}
            impl( const impl& t ) : subject_( t.subject_ )
                , description_( t.description_ )
                , items_( t.items_ ) {
            }
            std::string subject_;
            std::string description_;
            std::vector< MethodItem > items_;
            idAudit ident_;

        private:
            friend class boost::serialization::access;
            template<class Archive> void serialize( Archive& ar, const unsigned int version ) {
                ar & BOOST_SERIALIZATION_NVP( subject_ );
                ar & BOOST_SERIALIZATION_NVP( description_ );
                ar & BOOST_SERIALIZATION_NVP( items_ );
                if ( version >= 2 )
                    ar & BOOST_SERIALIZATION_NVP( ident_ );
            }

        };

        template< typename T = MethodItem > class MethodItem_archive {
        public:
            template<class Archive>
                void serialize( Archive& ar, T& _, const unsigned int version ) {
                using namespace boost::serialization;
                ar & BOOST_SERIALIZATION_NVP(_.modelname_)
                    & BOOST_SERIALIZATION_NVP(_.unitnumber_)
                    & BOOST_SERIALIZATION_NVP(_.isInitialCondition_)
                    & BOOST_SERIALIZATION_NVP(_.time_)
                    & BOOST_SERIALIZATION_NVP(_.funcid_)
                    & BOOST_SERIALIZATION_NVP(_.label_)
                    & BOOST_SERIALIZATION_NVP(_.data_)
                    ;
                if ( version >= 2 )
                    ar & BOOST_SERIALIZATION_NVP( _.description_ );
                if ( version >= 3 )
                    ar & BOOST_SERIALIZATION_NVP( _.clsid_ );
            }            
        };

        template<> ADCONTROLSSHARED_EXPORT void MethodItem::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
        {
            data_ = base64_encode( reinterpret_cast<const unsigned char *>( data_.data() ), data_.size() );
            MethodItem_archive<>().serialize( ar, *this, version );
        }
    
        template<> ADCONTROLSSHARED_EXPORT void MethodItem::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
        {
            MethodItem_archive<>().serialize( ar, *this, version );
            data_ = base64_decode( data_ );
        }
    
        template<> ADCONTROLSSHARED_EXPORT void MethodItem::serialize( portable_binary_oarchive& ar, const unsigned int version )
        {
            MethodItem_archive<>().serialize( ar, *this, version );
        }
    
        template<> ADCONTROLSSHARED_EXPORT void MethodItem::serialize( portable_binary_iarchive& ar, const unsigned int version )
        {
            MethodItem_archive<>().serialize( ar, *this, version );
        }        
    }
}

BOOST_CLASS_VERSION( adcontrols::ControlMethod::Method::impl, 2 )

namespace adcontrols {

    namespace ControlMethod {

        ////////// PORTABLE BINARY ARCHIVE //////////
        template<> void
        Method::serialize( portable_binary_oarchive& ar, const unsigned int )
        {
            ar & *impl_;
        }

        template<> void
        Method::serialize( portable_binary_iarchive& ar, const unsigned int version )
        {
            if ( version <= 1 ) {
                using namespace boost::serialization;
                ar & BOOST_SERIALIZATION_NVP( impl_->subject_ );
                ar & BOOST_SERIALIZATION_NVP( impl_->description_ );
                ar & BOOST_SERIALIZATION_NVP( impl_->items_ );
            } else
                ar & *impl_;
        }

        ///////// XML archive ////////
        template<> ADCONTROLSSHARED_EXPORT void
        Method::serialize( boost::archive::xml_woarchive& ar, const unsigned int )
        {
            ar & boost::serialization::make_nvp( "impl", *impl_ );
        }

        template<> ADCONTROLSSHARED_EXPORT void
        Method::serialize( boost::archive::xml_wiarchive& ar, const unsigned int )
        {
            ar & boost::serialization::make_nvp( "impl", *impl_ );
        }
    }
}

using namespace adcontrols::ControlMethod;

Method::~Method()
{
}

Method::Method() : impl_( new impl() )
{
}

Method::Method( const Method& t ) : impl_( new impl( *t.impl_ ) ) 
{
}

Method&
Method::operator = ( const Method & t )
{
    impl_.reset( new impl( *t.impl_ ) );
    return *this;
}

bool
Method::operator += ( const Method& rhs )
{
    bool modified( false );
    for ( const auto& item : rhs ) {
        auto it = std::find_if( begin(), end(), [item] ( const MethodItem& a ) { return item == a; } );
        if ( it != end() ) {
            modified = true;
            (*it) = item;
        }
        // nothing to be added if not in the lvalue
    }
    return modified;
}


Method::iterator
Method::begin()
{
    return impl_->items_.begin();
}

Method::iterator
Method::end()
{
    return impl_->items_.end();
}

Method::const_iterator
Method::begin() const
{
    return impl_->items_.begin();
}

Method::const_iterator
Method::end() const
{
    return impl_->items_.end();
}

Method::iterator
Method::erase( iterator pos )
{
    return impl_->items_.erase( pos );
}

Method::iterator
Method::erase( iterator first, iterator last )
{
    return impl_->items_.erase( first, last );
}

Method::iterator
Method::insert( const MethodItem& item )
{
    auto it = std::lower_bound( impl_->items_.begin(), impl_->items_.end(), item, []( const MethodItem& a, const MethodItem& b ){
            if ( adportable::compare<double>::essentiallyEqual( a.time(), b.time() ) ) {
                if ( a.modelname() == b.modelname() )
                    return a.unitnumber() < b.unitnumber();
                return a.modelname() < b.modelname();
            }
            return a.time() < b.time();
        });
    return impl_->items_.insert( it, item );
}

void
Method::push_back( const MethodItem& item )
{
    impl_->items_.push_back( item );
}

Method::iterator
Method::add( const MethodItem& item, bool uniq )
{
    if ( uniq ) {
        auto it = std::find_if( begin(), end(), [item] ( const MethodItem& a ) { return item == a; } );
        if ( it != end() )
            impl_->items_.erase( it );
    }
    return insert( item );
}


void
Method::sort()
{
    std::sort( impl_->items_.begin(), impl_->items_.end(), []( const MethodItem& a, const MethodItem& b ){
            if ( adportable::compare<double>::essentiallyEqual( a.time(), b.time() ) ) {
                if ( a.modelname() == b.modelname() )
                    return a.unitnumber() < b.unitnumber();
                return a.modelname() < b.modelname();
            }
            return a.time() < b.time();            
        });
}

void
Method::clear()
{
    impl_->items_.clear();
}

size_t
Method::size() const 
{
    return impl_->items_.size();
}

const char * 
Method::description() const
{
    return impl_->description_.c_str();
}

void
Method::setDescription( const char * t )
{
    impl_->description_ = t ? t : "";
}

const char *
Method::subject() const
{
    return impl_->subject_.c_str();
}

void
Method::setSubject( const char * t )
{
    impl_->subject_ = t ? t : "";
}

///////////////////////////////////////////////////////////

MethodItem::MethodItem() : unitnumber_( 0 )
                         , isInitialCondition_( true )
                         , time_( -1 )
                         , funcid_( 0 )
                         , clsid_( { 0 } )
{
}

MethodItem::MethodItem( const boost::uuids::uuid& clsid
                        , const std::string& model
                        , uint32_t unitnumber
                        , uint32_t funcid ) : modelname_( model )
                                            , unitnumber_( unitnumber )
                                            , isInitialCondition_( true )
                                            , time_( -1 )
                                            , funcid_( funcid )
                                            , clsid_( clsid )
{
}


MethodItem::MethodItem( const MethodItem& t ) : modelname_( t.modelname_ )
                                              , unitnumber_( t.unitnumber_ )
                                              , isInitialCondition_( t.isInitialCondition_ )
                                              , time_( t.time_ )
                                              , funcid_( t.funcid_ )
                                              , label_( t.label_ )
                                              , data_( t.data_ )
                                              , clsid_( t.clsid_ )
{
}

const boost::uuids::uuid&
MethodItem::clsid() const
{
    return clsid_;
}

void
MethodItem::setClsid( const boost::uuids::uuid& uuid )
{
    clsid_ = uuid;
}

bool
MethodItem::operator == ( const MethodItem& t ) const
{
    if ( isInitialCondition_ ) {
        return ( clsid_ == t.clsid_ ) && ( unitnumber_ == t.unitnumber_ ) && t.isInitialCondition_;
        // return modelname_ == t.modelname() && unitnumber_ == t.unitnumber() && isInitialCondition();
    } else {
        return ( clsid_ == t.clsid_ ) && ( unitnumber_ == t.unitnumber_ ) && adportable::compare<double>::essentiallyEqual( time_, t.time_ );
        // return modelname_ == t.modelname() && unitnumber_ == t.unitnumber() && adportable::compare<double>::essentiallyEqual( time_, t.time_ );
    }
}

const std::string&
MethodItem::modelname() const
{
    return modelname_;
}

void
MethodItem::setModelname( const char * value )
{
    modelname_ = value ? value : "";
}

uint32_t
MethodItem::unitnumber() const
{
    return unitnumber_;
}

void
MethodItem::unitnumber( uint32_t value ) 
{
    unitnumber_ = value;
}

bool
MethodItem::isInitialCondition() const
{
    return isInitialCondition_;
}

void
MethodItem::isInitialCondition( bool value )
{
    isInitialCondition_ = value;
    if ( isInitialCondition_ )
        time_ = (-1);
}

const double&
MethodItem::time() const
{
    return time_;
}

void
MethodItem::time( const double& value )
{
    isInitialCondition_ = false;
    time_ = value;
}

uint32_t
MethodItem::funcid() const
{
    return funcid_;
}

void
MethodItem::funcid( uint32_t value )
{
    funcid_ = value;
}

void
MethodItem::setItemLabel( const char * value )
{
    label_ = value ? value : "";
}

const std::string&
MethodItem::itemLabel() const
{
    return label_;
}

const char *
MethodItem::data() const
{
    return data_.data();
}

void
MethodItem::data( const char * data, size_t size )
{
    data_.resize( size );
    std::copy( data, data + size, data_.begin() );
}

const std::string&
MethodItem::description() const
{
    return description_;
}

void
MethodItem::setDescription( const char * data )
{
    description_ = data ? data : "";
}

size_t
MethodItem::size() const
{
    return data_.size();
}


bool
Method::archive( std::ostream& os, const Method& t )
{
    portable_binary_oarchive ar( os );
    try {
        ar << t;
        return true;
    } catch ( ... ) {
    }
    return false;
}

bool
Method::restore( std::istream& is, Method& t )
{
    portable_binary_iarchive ar( is );
    try {
        ar >> t;
        return true;
    } catch ( ... ) {
    }
    return false;
}

bool
Method::xml_archive( std::wostream& os, const Method& t )
{
    try {
        return internal::xmlSerializer( "Method" ).archive( os, t );
    } catch ( ... ) {
        return false;
    }
}

bool
Method::xml_restore( std::wistream& is, Method& t )
{
    try {
        return internal::xmlSerializer( "Method" ).restore( is, t );
    } catch ( ... ) {
        return false;
    }
}

Method::iterator
Method::find( iterator first, iterator last, const boost::uuids::uuid& clsid, int unitnumber )
{
    if ( unitnumber <= 0 ) {
        return std::find_if( first, last, [&]( const MethodItem& a ){ return a.clsid() == clsid; });
    } else {
        return std::find_if( first, last, [&]( const MethodItem& a ){
                return ( a.clsid() == clsid ) && ( a.unitnumber() == unitnumber );
            });
    }
}

Method::const_iterator
Method::find( const_iterator first, const_iterator last, const boost::uuids::uuid& clsid, int unitnumber ) const
{
    if ( unitnumber <= 0 ) {
        return std::find_if( first, last, [&]( const MethodItem& a ){ return a.clsid() == clsid; });
    } else {
        return std::find_if( first, last, [&]( const MethodItem& a ){
                return ( a.clsid() == clsid ) && ( a.unitnumber() == unitnumber );
            });
    }
}

template<> const MethodItem *
any_cast<const MethodItem *>::operator()( boost::any& a, const boost::uuids::uuid& clsid ) const
{
    try {
        auto pi = boost::any_cast<const MethodItem *>( a );
        return pi;
    } catch ( boost::bad_any_cast& ) {
    }
    
    try {
        if ( auto ptr = boost::any_cast<std::shared_ptr< const Method > >( a ) ) {
            auto it = ptr->find( ptr->begin(), ptr->end(), clsid );
            if ( it != ptr->end() )
                return &( *it );
        }
    } catch ( boost::bad_any_cast& ) {
    }
    return nullptr;
}

template<> MethodItem *
any_cast<MethodItem *>::operator()( boost::any& a, const boost::uuids::uuid& clsid ) const
{
    try {
        auto pi = boost::any_cast< MethodItem *>( a );
        return pi;
    } catch ( boost::bad_any_cast& ) {
    }
    try {
        if ( auto ptr = boost::any_cast<std::shared_ptr< Method > >( a ) ) {
            auto it = ptr->find( ptr->begin(), ptr->end(), clsid );
            if ( it != ptr->end() )
                return &( *it );
        }
    } catch ( boost::bad_any_cast& ) {
    }
    return nullptr;
}

