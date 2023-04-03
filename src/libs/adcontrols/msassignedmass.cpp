// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC
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

#include "msassignedmass.hpp"
#include "msproperty.hpp"
#include "serializer.hpp"
#include "samplinginfo.hpp"
#include <adportable/float.hpp>
#include <adportable/utf.hpp>
#include <boost/bind.hpp>
#include <algorithm>

namespace adcontrols {

    class MSAssignedMass::impl {
    public:
        ~impl() {}
        impl() : idReference_(-1)
               , idMassSpectrum_(0)
               , idPeak_(-1)
               , exactMass_( 0 )
               , time_( 0 )
               , mass_( 0 )
               , enable_( false )
               , flags_( 0 )
               , mode_( 0 ) {
        }
        impl( const impl& t ) : formula_( t.formula_ )
                              , idReference_( t.idReference_ )
                              , idMassSpectrum_( t.idMassSpectrum_ )
                              , idPeak_( t.idPeak_ )
                              , exactMass_( t.exactMass_ )
                              , time_( t.time_ )
                              , mass_( t.mass_ )
                              , enable_( t.enable_ )
                              , flags_( t.flags_ )
                              , mode_( t.mode_ ) {
        }

        std::string formula_;
        uint32_t idReference_;
        uint32_t idMassSpectrum_; // segment# on segment_wrapper<MassSpectrum>[]
        uint32_t idPeak_;         // peak# on MassSpectrum
        double exactMass_;
        double time_;
        double mass_;
        bool enable_;  // if false, this peak does not use for polynomial fitting
        uint32_t flags_;
        uint32_t mode_; // number of turns for InfiTOF, linear|reflectron for MALDI and/or any analyzer mode

        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const uint32_t version) {
            using namespace boost::serialization;
            if ( version >= 4 ) {
                ar & BOOST_SERIALIZATION_NVP( formula_);
                ar & BOOST_SERIALIZATION_NVP( idReference_ );
                ar & BOOST_SERIALIZATION_NVP( idMassSpectrum_);
                ar & BOOST_SERIALIZATION_NVP( idPeak_);
                ar & BOOST_SERIALIZATION_NVP( exactMass_);
                ar & BOOST_SERIALIZATION_NVP( time_);
                ar & BOOST_SERIALIZATION_NVP( mass_);
                ar & BOOST_SERIALIZATION_NVP( enable_);
                ar & BOOST_SERIALIZATION_NVP( flags_ );
                ar & BOOST_SERIALIZATION_NVP( mode_ );
            }
            if ( version == 3 )  {
                std::wstring wformula;
                ar & BOOST_SERIALIZATION_NVP( wformula );
                ar & BOOST_SERIALIZATION_NVP( idReference_ );
                ar & BOOST_SERIALIZATION_NVP( idMassSpectrum_);
                ar & BOOST_SERIALIZATION_NVP( idPeak_);
                ar & BOOST_SERIALIZATION_NVP( exactMass_);
                ar & BOOST_SERIALIZATION_NVP( time_);
                ar & BOOST_SERIALIZATION_NVP( mass_);
                ar & BOOST_SERIALIZATION_NVP( enable_);
                ar & BOOST_SERIALIZATION_NVP( flags_ );
                ar & BOOST_SERIALIZATION_NVP( mode_ );
                if ( Archive::is_loading::value ) {
                    formula_ = adportable::utf::to_utf8( wformula );
                }
            }
            if ( version < 3 )
                return; // ignore
        }
    };

}

using namespace adcontrols;

MSAssignedMass::~MSAssignedMass()
{
}

MSAssignedMass::MSAssignedMass() : impl_( std::make_unique< impl >() )
{
}

MSAssignedMass::MSAssignedMass( const MSAssignedMass& t )
    : impl_( std::make_unique< impl >( *t.impl_ ) )
{
}

const MSAssignedMass&
MSAssignedMass::operator = ( const MSAssignedMass& t )
{
    impl_ = std::make_unique< impl >( *t.impl_ );
    return *this;
}

MSAssignedMass::MSAssignedMass( uint32_t idReference
                                , uint32_t idMassSpectrum
								, uint32_t idPeak
                                , const std::string& formula
                                , double exactMass
                                , double time
                                , double mass
                                , bool enable
                                , uint32_t flags
                                , uint32_t mode ) : impl_( std::make_unique< impl >() )
{
    impl_->idReference_       = idReference;
    impl_->idMassSpectrum_    = idMassSpectrum;
    impl_->idPeak_            = idPeak;
    impl_->formula_           = formula;
    impl_->exactMass_         = exactMass;
    impl_->time_              = time;
    impl_->mass_              = mass;
    impl_->enable_            = enable;
    impl_->flags_             = flags;
    impl_->mode_              = mode;
}

uint32_t
MSAssignedMass::idReference() const
{
    return impl_->idReference_;
}

void
MSAssignedMass::idReference( uint32_t value )
{
    impl_->idReference_ = value;
}

uint32_t
MSAssignedMass::idMassSpectrum() const
{
    return impl_->idMassSpectrum_;
}

void
MSAssignedMass::idMassSpectrum( uint32_t value )
{
	impl_->idMassSpectrum_ = value;
}

uint32_t
MSAssignedMass::idPeak() const
{
    return impl_->idPeak_;
}

void
MSAssignedMass::idPeak( uint32_t value )
{
	impl_->idPeak_ = value;
}

double
MSAssignedMass::exactMass() const
{
    return impl_->exactMass_;
}

void
MSAssignedMass::exactMass( double value )
{
    impl_->exactMass_ = value;
}

double
MSAssignedMass::time() const
{
    return impl_->time_;
}

void
MSAssignedMass::time( double value )
{
    impl_->time_ = value;
}

double
MSAssignedMass::mass() const
{
    return impl_->mass_;
}

void
MSAssignedMass::mass( double value )
{
    impl_->mass_ = value;
}

bool
MSAssignedMass::enable() const
{
    return impl_->enable_;
}

void
MSAssignedMass::enable( bool value )
{
    impl_->enable_ = value;
}

uint32_t
MSAssignedMass::flags() const
{
    return impl_->flags_;
}

uint32_t
MSAssignedMass::mode() const
{
    return impl_->mode_;
}

void
MSAssignedMass::flags( uint32_t value )
{
    impl_->flags_ = value;
}

void
MSAssignedMass::mode( uint32_t value )
{
    impl_->mode_ = value;
}

std::string
MSAssignedMass::formula() const
{
    return impl_->formula_;
}

void
MSAssignedMass::formula( const std::wstring& value )
{
    impl_->formula_ = adportable::utf::to_utf8( value );
}

void
MSAssignedMass::formula( const std::string& value )
{
    impl_->formula_ = value;
}

/////////////////////

MSAssignedMasses::MSAssignedMasses()
{
}

MSAssignedMasses::MSAssignedMasses( const MSAssignedMasses& t ) : vec_( t.vec_ )
{
}

MSAssignedMasses&
MSAssignedMasses::operator << ( const MSAssignedMass& t )
{
    vec_.push_back( t );
    return *this;
}

bool
MSAssignedMasses::operator += ( const MSAssignedMass& t )
{
    if ( std::find_if( vec_.begin(), vec_.end(), [t]( const MSAssignedMass& a ){
                return t.mode() == a.mode() &&
                    adportable::compare<double>::essentiallyEqual( t.exactMass(), a.exactMass() );
            }) == vec_.end() ) {
        vec_.push_back( t );
        return true;
    }
    return false;
}

MSAssignedMasses&
MSAssignedMasses::operator += ( const MSAssignedMasses& t )
{
    for ( vector_type::const_iterator it = t.begin(); it != t.end(); ++it )
        *this += *it;
    return *this;
}

MSAssignedMasses::vector_type::iterator
MSAssignedMasses::begin()
{
    return vec_.begin();
}

MSAssignedMasses::vector_type::iterator
MSAssignedMasses::end()
{
    return vec_.end();
}

MSAssignedMasses::vector_type::const_iterator
MSAssignedMasses::begin() const
{
    return vec_.begin();
}

MSAssignedMasses::vector_type::const_iterator
MSAssignedMasses::end() const
{
    return vec_.end();
}

size_t
MSAssignedMasses::size() const
{
    return vec_.size();
}

namespace adcontrols {

    template<> void
    MSAssignedMass::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        // saving
        impl_->serialize( ar, version );
    }

    template<> void
    MSAssignedMass::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        // loading
        impl_->serialize( ar, version );
    }

    ///////// XML archive ////////
    template<> void
    MSAssignedMass::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        // saving
        impl_->serialize( ar, version );
    }

    template<> void
    MSAssignedMass::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        impl_->serialize( ar, version );
    }
}
