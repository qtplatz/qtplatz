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
#include <adportable/float.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <workaround/boost/uuid/uuid_serialize.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/archive/xml_woarchive.hpp>

namespace adcontrols {
    
    class ControlMethod::impl {
    public:
        impl() {}
        impl( const impl& t ) : subject_( t.subject_ )
                              , description_( t.description_ )
                              , items_( t.items_ ) {
        }
        std::string subject_;
        std::string description_;
        std::vector< controlmethod::MethodItem > items_;
        idAudit ident_;

    private:
        friend class boost::serialization::access;
        template<class Archive> void serialize(Archive& ar, const unsigned int version ) {
            ar & BOOST_SERIALIZATION_NVP( subject_ );
            ar & BOOST_SERIALIZATION_NVP( description_ );
            ar & BOOST_SERIALIZATION_NVP( items_ );
            if ( version >= 2 )
                ar & BOOST_SERIALIZATION_NVP( ident_ );
        }
        
    };

}

BOOST_CLASS_VERSION( adcontrols::ControlMethod::impl, 2 )

namespace adcontrols {

    ////////// PORTABLE BINARY ARCHIVE //////////
    template<> void
    ControlMethod::serialize( portable_binary_oarchive& ar, const unsigned int )
    {
        ar & *impl_;
    }

    template<> void
    ControlMethod::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        if ( version <= 1 )  {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( impl_->subject_ );
            ar & BOOST_SERIALIZATION_NVP( impl_->description_ );
            ar & BOOST_SERIALIZATION_NVP( impl_->items_ );
        } else
            ar & *impl_;
    }

    ///////// XML archive ////////
    template<> ADCONTROLSSHARED_EXPORT void
    ControlMethod::serialize( boost::archive::xml_woarchive& ar, const unsigned int )
    {
        ar & boost::serialization::make_nvp("impl", *impl_);
    }

    template<> ADCONTROLSSHARED_EXPORT void
    ControlMethod::serialize( boost::archive::xml_wiarchive& ar, const unsigned int )
    {
        ar & boost::serialization::make_nvp( "impl", *impl_ );
    }
}

using namespace adcontrols;

ControlMethod::~ControlMethod()
{
}

ControlMethod::ControlMethod() : impl_( new impl() )
{
}

ControlMethod::ControlMethod( const ControlMethod& t ) : impl_( new impl( *t.impl_ ) ) 
{
}

ControlMethod&
ControlMethod::operator = ( const ControlMethod & t )
{
    impl_.reset( new impl( *t.impl_ ) );
    return *this;
}

ControlMethod::iterator
ControlMethod::begin()
{
    return impl_->items_.begin();
}

ControlMethod::iterator
ControlMethod::end()
{
    return impl_->items_.end();
}

ControlMethod::const_iterator
ControlMethod::begin() const
{
    return impl_->items_.begin();
}

ControlMethod::const_iterator
ControlMethod::end() const
{
    return impl_->items_.end();
}

ControlMethod::iterator
ControlMethod::erase( iterator pos )
{
    return impl_->items_.erase( pos );
}

ControlMethod::iterator
ControlMethod::erase( iterator first, iterator last )
{
    return impl_->items_.erase( first, last );
}

ControlMethod::iterator
ControlMethod::insert( const controlmethod::MethodItem& item )
{
    using adcontrols::controlmethod::MethodItem;

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
ControlMethod::push_back( const controlmethod::MethodItem& item )
{
    impl_->items_.push_back( item );
}

void
ControlMethod::sort()
{
    using adcontrols::controlmethod::MethodItem;

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
ControlMethod::clear()
{
    impl_->items_.clear();
}

size_t
ControlMethod::size() const 
{
    return impl_->items_.size();
}

const char * 
ControlMethod::description() const
{
    return impl_->description_.c_str();
}

void
ControlMethod::setDescription( const char * t )
{
    impl_->description_ = t ? t : "";
}

const char *
ControlMethod::subject() const
{
    return impl_->subject_.c_str();
}

void
ControlMethod::setSubject( const char * t )
{
    impl_->subject_ = t ? t : "";
}

using namespace adcontrols::controlmethod;

MethodItem::MethodItem() : unitnumber_( 0 )
                         , isInitialCondition_( true )
                         , time_( -1 )
                         , funcid_( 0 )
{
}

MethodItem::MethodItem( const MethodItem& t ) : modelname_( t.modelname_ )
                                              , unitnumber_( t.unitnumber_ )
                                              , isInitialCondition_( t.isInitialCondition_ )
                                              , time_( t.time_ )
                                              , funcid_( t.funcid_ )
                                              , label_( t.label_ )
                                              , data_( t.data_ )
{
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
ControlMethod::archive( std::ostream& os, const ControlMethod& t )
{
    portable_binary_oarchive ar( os );
    ar << t;
    return true;
}

bool
ControlMethod::restore( std::istream& is, ControlMethod& t )
{
    portable_binary_iarchive ar( is );
    ar >> t;
    return true;
}

bool
ControlMethod::xml_archive( std::wostream& os, const ControlMethod& t )
{
    return internal::xmlSerializer("ControlMethod").archive( os, t );
}

bool
ControlMethod::xml_restore( std::wistream& is, ControlMethod& t )
{
    return internal::xmlSerializer("ControlMethod").restore( is, t );
}

