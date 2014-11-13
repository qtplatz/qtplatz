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
#if defined _MSC_VER
# pragma warning (disable:4996)
#endif

#include "quancompounds.hpp"
#include "serializer.hpp"
#include <adportable/uuid.hpp>
#include <boost/serialization/vector.hpp>

#if defined _MSC_VER
# pragma warning (default:4996)
#endif

namespace adcontrols {
    
    class QuanCompounds::impl {
    public:
        impl() {
        }

        impl( const impl& t ) : ident_( t.ident_ )
                              , compounds_( t.compounds_ ) {
        }

    public:
        idAudit ident_;
        std::vector< QuanCompound > compounds_;

        //private:
        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int version ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( ident_ );
            ar & BOOST_SERIALIZATION_NVP( compounds_ );
        }
    };
}

BOOST_CLASS_VERSION( adcontrols::QuanCompounds::impl, 2 )

namespace adcontrols {

    ////////// PORTABLE BINARY ARCHIVE //////////
    template<> void
    QuanCompounds::serialize( portable_binary_oarchive& ar, const unsigned int )
    {
        ar << *impl_;
    }

    template<> void
    QuanCompounds::serialize( portable_binary_iarchive& ar, const unsigned int version)
    {
        if ( version >= 2 )
            ar >> *impl_;
        else
            impl_->serialize( ar, version );
    }

    ///////// XML archive ////////
    template<> ADCONTROLSSHARED_EXPORT void
    QuanCompounds::serialize( boost::archive::xml_woarchive& ar, const unsigned int )
    {
        ar & boost::serialization::make_nvp("QuanCompounds", *impl_);
    }

    template<> ADCONTROLSSHARED_EXPORT void
    QuanCompounds::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        if ( version >= 2 )
            ar & boost::serialization::make_nvp( "QuanCompounds", *impl_ );
        else
            impl_->serialize( ar, version );
    }
}

using namespace adcontrols;

QuanCompounds::~QuanCompounds()
{
}

QuanCompounds::QuanCompounds() : impl_( new impl() )
{
}

QuanCompounds::QuanCompounds( const QuanCompounds& t ) : impl_( new impl( *t.impl_ ) )
{
}

QuanCompounds&
QuanCompounds::operator = ( const QuanCompounds& t )
{
    impl_.reset( new impl( *t.impl_ )  );
    return *this;
}

QuanCompounds&
QuanCompounds::operator << ( const QuanCompound& t )
{
    impl_->compounds_.push_back( t );
    impl_->compounds_.back().row( uint32_t( impl_->compounds_.size() - 1 ) );
    return *this;
}

const boost::uuids::uuid&
QuanCompounds::uuid() const
{
    return impl_->ident_.uuid();
}


QuanCompounds::iterator QuanCompounds::begin()
{
    return impl_->compounds_.begin();
}

QuanCompounds::iterator QuanCompounds::end()
{
    return impl_->compounds_.end();
}

QuanCompounds::const_iterator QuanCompounds::begin() const
{
    return impl_->compounds_.begin();
}

QuanCompounds::const_iterator QuanCompounds::end() const
{
    return impl_->compounds_.end();
}

void
QuanCompounds::clear()
{
    impl_->compounds_.clear();
}

size_t
QuanCompounds::size() const
{
    return impl_->compounds_.size();
}

const idAudit&
QuanCompounds::ident() const
{
    return impl_->ident_;
}

// static
bool
QuanCompounds::xml_archive( std::wostream& os, const QuanCompounds& t ) 
{
    return internal::xmlSerializer( "QuanCompounds" ).archive( os, t );
}

// static
bool
QuanCompounds::xml_restore( std::wistream& is, QuanCompounds& t ) 
{
    return internal::xmlSerializer( "QuanCompounds" ).restore( is, t );
}

