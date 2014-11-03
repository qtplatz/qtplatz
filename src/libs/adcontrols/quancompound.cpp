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
#pragma warning (disable:4996)
#endif
#include "quancompound.hpp"
#include "serializer.hpp"
#include <adportable/uuid.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/uuid/uuid_serialize.hpp>
#include <boost/exception/all.hpp>

#include <array>
#include <string>
#include <vector>
#include <cstdint>

namespace adcontrols {
    
    class QuanCompound::impl {
    public:
        ~impl() {
        }
  
        impl() : uuid_( adportable::uuid()() )
               , row_( 0 ) {
        }

        impl( const boost::uuids::uuid& uuid ) : uuid_( uuid )
                                               , row_( 0 )
                                               , amounts_( 1 )
                                               , tR_( 0 )
                                               , mass_( 0 )
                                               , isISTD_( false )
                                               , isLKMSRef_( false )
                                               , isTimeRef_( false )
                                               , idISTD_( -1 )
                                               , criteria_( std::make_pair( 0, 0 ) ) {
        }

        impl( const impl& t ) : uuid_( t.uuid_ )
                              , row_( t.row_ )
                              , display_name_( t.display_name_ )
                              , formula_( t.formula_ )
                              , amounts_( t.amounts_ )
                              , description_( t.description_ )
                              , tR_( t.tR_ )
                              , mass_( t.mass_ )
                              , isISTD_( t.isISTD_ )
                              , isLKMSRef_( t.isLKMSRef_ )
                              , isTimeRef_( t.isTimeRef_ )
                              , idISTD_( t.idISTD_ )
                              , criteria_( t.criteria_ ) {
        }

    public:
        boost::uuids::uuid uuid_;
        int32_t row_;                    // row# in Compounds, useful for report order by 'row'
        std::wstring display_name_;
        std::string formula_;
        std::vector< double > amounts_;  // added amounts[ level ]
        std::wstring description_;
        double tR_;
        double mass_;
        bool isISTD_;     // am I an internal standard?
        bool isLKMSRef_;
        bool isTimeRef_;
        int32_t idISTD_;  // index for internal standad (referenced from non-istd
        std::pair< double, double > criteria_;  // pass/fail criteria

    //private:
        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int version ) {
            using namespace boost::serialization;

            ar & BOOST_SERIALIZATION_NVP( uuid_ );
            ar & BOOST_SERIALIZATION_NVP( row_ );
            ar & BOOST_SERIALIZATION_NVP( formula_ );
            ar & BOOST_SERIALIZATION_NVP( display_name_ );
            ar & BOOST_SERIALIZATION_NVP( amounts_ );
            ar & BOOST_SERIALIZATION_NVP( description_ );
            ar & BOOST_SERIALIZATION_NVP( isISTD_ );
            ar & BOOST_SERIALIZATION_NVP( isLKMSRef_ );
            ar & BOOST_SERIALIZATION_NVP( isTimeRef_ );
            ar & BOOST_SERIALIZATION_NVP( idISTD_ );
            ar & BOOST_SERIALIZATION_NVP( tR_ );
            ar & BOOST_SERIALIZATION_NVP( mass_ );
            ar & BOOST_SERIALIZATION_NVP( criteria_ );

        }
        
    };
}

BOOST_CLASS_VERSION( adcontrols::QuanCompound::impl, 2 )

namespace adcontrols {

    ////////// PORTABLE BINARY ARCHIVE //////////
    template<> void
    QuanCompound::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        ar & *impl_;
    }

    template<> void
    QuanCompound::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        if ( version >= 2 )
            ar & *impl_;
        else
            impl_->serialize( ar, version );
    }

    ///////// XML archive ////////
    template<> void
    QuanCompound::serialize( boost::archive::xml_woarchive& ar, const unsigned int )
    {
        ar & boost::serialization::make_nvp( "impl", *impl_);
    }

    template<> void
    QuanCompound::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        if ( version >= 2 )
            ar & boost::serialization::make_nvp( "impl", *impl_);
        else
            impl_->serialize( ar, version );
    }
}


using namespace adcontrols;

QuanCompound::~QuanCompound()
{
}

QuanCompound::QuanCompound( const boost::uuids::uuid& uuid ) : impl_( new impl( uuid ) )
{
}

QuanCompound::QuanCompound() : impl_( new impl() )
{
}

QuanCompound::QuanCompound( const QuanCompound& t ) : impl_( new impl( *t.impl_ ) )
{
}

QuanCompound&
QuanCompound::operator = ( const QuanCompound& t )
{
    impl_.reset( new impl( *t.impl_ )  );
    return *this;
}

QuanCompound
QuanCompound::null()
{
    static boost::uuids::uuid uuid; // all zero
    return QuanCompound( uuid );
}

const boost::uuids::uuid&
QuanCompound::uuid() const
{
    return impl_->uuid_; // 'refCmpd'
}

uint32_t
QuanCompound::row() const
{
    return impl_->row_;
}

void
QuanCompound::row( uint32_t t )
{
    impl_->row_ = t;
}

const wchar_t *
QuanCompound::display_name() const
{
    return impl_->display_name_.c_str();
}

void
QuanCompound::displya_name( const wchar_t * v )
{
    impl_->display_name_ = v;
}

const wchar_t *
QuanCompound::description() const
{
    return impl_->description_.c_str();
}

void
QuanCompound::description( const wchar_t * v )
{
    impl_->description_ = v;
}

const char *
QuanCompound::formula() const
{
    return impl_->formula_.c_str();
}

void
QuanCompound::formula( const char * v )
{
    impl_->formula_ = v;
}

bool
QuanCompound::isLKMSRef() const
{
    return impl_->isLKMSRef_;
}

void
QuanCompound::isLKMSRef( bool f )
{
    impl_->isLKMSRef_ = f;
}

bool
QuanCompound::isTimeRef() const
{
    return impl_->isTimeRef_;
}

void
QuanCompound::isTimeRef( bool f )
{
    impl_->isTimeRef_ = f;
}

bool
QuanCompound::isISTD() const
{
    return impl_->isISTD_;
}

void
QuanCompound::isISTD( bool f )
{
    impl_->isISTD_ = f;
}

int32_t
QuanCompound::idISTD() const
{
    return impl_->idISTD_;
}

void
QuanCompound::idISTD( int32_t v )
{
    impl_->idISTD_ = v;
}

void
QuanCompound::levels( size_t v )
{
    impl_->amounts_.resize(v);
}

double
QuanCompound::mass() const
{
    return impl_->mass_;
}

void
QuanCompound::mass( double v )
{
    impl_->mass_ = v;
}

double
QuanCompound::tR() const
{
    return impl_->tR_;
}

void
QuanCompound::tR( double v )
{
    impl_->tR_ = v;
};

size_t
QuanCompound::levels() const
{
    return impl_->amounts_.size();
}

const double *
QuanCompound::amounts() const
{
    return impl_->amounts_.data();
}

void
QuanCompound::amounts( const double * d, size_t size )
{
    if ( size != impl_->amounts_.size() )
        impl_->amounts_.resize( size );
    std::copy( d, d + size, impl_->amounts_.begin() );
}

double
QuanCompound::criteria( bool second ) const
{
    return second ? impl_->criteria_.second : impl_->criteria_.first;
}

void
QuanCompound::criteria( double v, bool second )
{
    if ( second )
        impl_->criteria_.second = v;
    else
        impl_->criteria_.first = v;
}

