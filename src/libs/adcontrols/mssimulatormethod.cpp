/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "mssimulatormethod.hpp"
#include "moltable.hpp"
#include "serializer.hpp"
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/utility.hpp>

#include <array>
#include <adportable/float.hpp>

namespace adcontrols {

    class MSSimulatorMethod::impl {
    public:
        std::pair< double, double > mass_limits_; // lower, upper
        std::pair< uint32_t, uint32_t > charge_state_; // min, max
        double resolving_power_;
        bool is_polarity_positive_;
        moltable molecules_;
        bool is_tof_;
        double tof_length_;
        double tof_accelerator_voltage_;
        double tof_tDelay_;
        
        friend class boost::serialization::access;
        template<class Archive>
        void serialize( Archive& ar, const unsigned int version ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( mass_limits_ );
            ar & BOOST_SERIALIZATION_NVP( charge_state_ );
            ar & BOOST_SERIALIZATION_NVP( resolving_power_ );
            ar & BOOST_SERIALIZATION_NVP( is_polarity_positive_ );
            ar & BOOST_SERIALIZATION_NVP( is_tof_ );
            ar & BOOST_SERIALIZATION_NVP( tof_length_ );
            ar & BOOST_SERIALIZATION_NVP( tof_accelerator_voltage_ );
            ar & BOOST_SERIALIZATION_NVP( tof_tDelay_ );
            ar & BOOST_SERIALIZATION_NVP( molecules_ );
        }
        
        impl() : mass_limits_( -1, -1 )
               , charge_state_( 1, 1 )
               , is_polarity_positive_( true )
               , resolving_power_( 10000.0 )
               , is_tof_( true )
               , tof_length_( 0.5 )
               , tof_accelerator_voltage_( 5000.0 )
               , tof_tDelay_( 0.0 )  {
        }

        impl( const impl& t ) : mass_limits_( t.mass_limits_ )
                              , charge_state_( t.charge_state_ )
                              , resolving_power_( t.resolving_power_ )
                              , is_polarity_positive_( t.is_polarity_positive_ )
                              , is_tof_( t.is_tof_ )
                              , tof_length_( t.tof_length_ )
                              , tof_accelerator_voltage_( t.tof_accelerator_voltage_ )
                              , tof_tDelay_( t.tof_tDelay_ )
                              , molecules_( t.molecules_ )  {
        }
    };
}

BOOST_CLASS_VERSION( adcontrols::MSSimulatorMethod::impl, 1 )

namespace adcontrols {

    ////////// PORTABLE BINARY ARCHIVE //////////
    template<> void
    MSSimulatorMethod::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        ar & *impl_;
    }

    template<> void
    MSSimulatorMethod::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        ar & *impl_;
    }

    ///////// XML archive ////////
    template<> void
    MSSimulatorMethod::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        ar & boost::serialization::make_nvp( "impl", *impl_ );
    }

    template<> void
    MSSimulatorMethod::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        ar & boost::serialization::make_nvp( "impl", *impl_ );
    }
}

using namespace adcontrols;

MSSimulatorMethod::~MSSimulatorMethod()
{
}

MSSimulatorMethod::MSSimulatorMethod() : impl_( new impl() )
{
}

MSSimulatorMethod::MSSimulatorMethod( const MSSimulatorMethod& t ) : impl_( new impl( *t.impl_ ) )
{
}

MSSimulatorMethod&
MSSimulatorMethod::operator = ( const MSSimulatorMethod& t )
{
    impl_.reset( new impl( *t.impl_ ) );
    return *this;
}

double
MSSimulatorMethod::lower_limit() const
{
    return impl_->mass_limits_.first;
}

double
MSSimulatorMethod::upper_limit() const
{
    return impl_->mass_limits_.second;
}

void
MSSimulatorMethod::set_lower_limit( double v )
{
    impl_->mass_limits_.first = v;
}

void
MSSimulatorMethod::set_upper_limit( double v )
{
    impl_->mass_limits_.second = v;
}

uint32_t
MSSimulatorMethod::charge_state_min() const
{
    return impl_->charge_state_.first;
}

uint32_t
MSSimulatorMethod::charge_state_max() const
{
    return impl_->charge_state_.second;
}

void
MSSimulatorMethod::set_charge_state_min( uint32_t value )
{
    impl_->charge_state_.first = value;
}

void
MSSimulatorMethod::set_charge_state_max( uint32_t value )
{
    impl_->charge_state_.second = value;    
}
        
void
MSSimulatorMethod::set_resolving_power( double value )
{
    impl_->resolving_power_ = value;
}

double
MSSimulatorMethod::resolving_power() const
{
    return impl_->resolving_power_;
}


bool
MSSimulatorMethod::is_positive_polarity() const
{
    return impl_->is_polarity_positive_;
}

void
MSSimulatorMethod::set_is_positive_polarity( bool value )
{
    impl_->is_polarity_positive_ = value;
}

const moltable&
MSSimulatorMethod::molecules() const
{
    return impl_->molecules_;
}

moltable&
MSSimulatorMethod::molecules()
{
    return impl_->molecules_;
}

void
MSSimulatorMethod::setMolecules( const moltable& value )
{
    impl_->molecules_ = value;
}

bool
MSSimulatorMethod::is_tof() const
{
    return impl_->is_tof_;
}

void
MSSimulatorMethod::set_is_tof( bool value )
{
    impl_->is_tof_ = value;    
}

double
MSSimulatorMethod::length() const
{
    return impl_->tof_length_;
}

void
MSSimulatorMethod::set_length( double value )
{
    impl_->tof_length_ = value;
}

double
MSSimulatorMethod::accelerator_voltage() const
{
    return impl_->tof_accelerator_voltage_;
}

void
MSSimulatorMethod::set_accelerator_voltage( double value )
{
    impl_->tof_accelerator_voltage_ = value;
}
        
double
MSSimulatorMethod::tDelay() const
{
    return impl_->tof_tDelay_;
}

void
MSSimulatorMethod::set_tDelay( double value )
{
    impl_->tof_tDelay_ = value;
}
        
