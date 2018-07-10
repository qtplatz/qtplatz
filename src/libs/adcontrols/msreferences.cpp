// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "msreferences.hpp"
#include "msreference.hpp"
#include "serializer.hpp"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/version.hpp>

namespace adcontrols {

    class MSReferences::impl {
    public:
        impl() { }
        impl( const impl& t ) : vec_(t.vec_)
                              , name_(t.name_) {
        }

        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            using namespace boost::serialization;
            (void)version;
            ar & BOOST_SERIALIZATION_NVP(name_);
            ar & BOOST_SERIALIZATION_NVP(vec_);
        }
        
        vector_type vec_;
        std::wstring name_;
    };
}


namespace adcontrols {

    ////////// PORTABLE BINARY ARCHIVE //////////
    template<> void
    MSReferences::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        impl_->serialize( ar, version );
    }

    template<> void
    MSReferences::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        impl_->serialize( ar, version );
    }

    ///////// XML archive ////////
    template<> void
    MSReferences::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        impl_->serialize( ar, version );
    }

    template<> void
    MSReferences::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        impl_->serialize( ar, version );
    }
}


using namespace adcontrols;


MSReferences::~MSReferences()
{
}

MSReferences::MSReferences() : impl_( new impl )
{
}

MSReferences::MSReferences( const MSReferences& t ) : impl_( new impl( *t.impl_ ) )
{
}

MSReferences&
MSReferences::operator = ( const MSReferences& t )
{
    impl_.reset( new impl( *t.impl_ ) );
    return *this;
}

MSReferences::vector_type::iterator
MSReferences::begin()
{
    return impl_->vec_.begin();
}

MSReferences::vector_type::iterator
MSReferences::end()
{
    return impl_->vec_.end();
}

MSReferences::vector_type::const_iterator
MSReferences::begin() const
{
    return impl_->vec_.begin();
}

MSReferences::vector_type::const_iterator
MSReferences::end() const
{
    return impl_->vec_.end();
}

const wchar_t *
MSReferences::name() const
{
    return impl_->name_.c_str();
}

void
MSReferences::name( const wchar_t * name )
{
    impl_->name_ = name ? name : L"";
}

void
MSReferences::clear()
{
    impl_->vec_.clear();
}

size_t
MSReferences::size() const
{
    return impl_->vec_.size();
}

const MSReference&
MSReferences::operator [] ( int idx ) const
{
    return impl_->vec_[ idx ];
}

MSReference&
MSReferences::operator [] ( int idx )
{
    return impl_->vec_[ idx ];
}

MSReferences&
MSReferences::operator << ( const MSReference& t )
{
    impl_->vec_.push_back( t );
    return *this;
}
