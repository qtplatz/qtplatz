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
        int protocol_;
        int mode_;
        double abundance_low_limit_;

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
            if ( version >= 2 ) {
                ar & BOOST_SERIALIZATION_NVP( protocol_ );
                ar & BOOST_SERIALIZATION_NVP( mode_ );
            }
            if ( version >= 3 ) {
                ar & BOOST_SERIALIZATION_NVP( abundance_low_limit_ );
            }
        }

        impl() : mass_limits_( -1, -1 )
               , charge_state_( 1, 1 )
               , resolving_power_( 10000.0 )
               , is_polarity_positive_( true )
               , is_tof_( true )
               , tof_length_( 0.5 )
               , tof_accelerator_voltage_( 5000.0 )
               , tof_tDelay_( 0.0 )
               , protocol_( 0 )
               , mode_( 0 )
               , abundance_low_limit_( 0.1 ) {
        }

        impl( const impl& t ) : mass_limits_( t.mass_limits_ )
                              , charge_state_( t.charge_state_ )
                              , resolving_power_( t.resolving_power_ )
                              , is_polarity_positive_( t.is_polarity_positive_ )
                              , molecules_( t.molecules_ )
                              , is_tof_( t.is_tof_ )
                              , tof_length_( t.tof_length_ )
                              , tof_accelerator_voltage_( t.tof_accelerator_voltage_ )
                              , tof_tDelay_( t.tof_tDelay_ )
                              , protocol_( t.protocol_ )
                              , mode_( t.mode_ )
                              , abundance_low_limit_( t.abundance_low_limit_ ) {
        }
    };
}

BOOST_CLASS_VERSION( adcontrols::MSSimulatorMethod::impl, 3 )

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
        try {
            ar & boost::serialization::make_nvp( "impl", *impl_ );
        } catch ( std::exception& ) {
            BOOST_THROW_EXCEPTION( serializer_error() << info( "serialize to xml_woarchive" ) );
        }

    }

    template<> void
    MSSimulatorMethod::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        try {
            ar & boost::serialization::make_nvp( "impl", *impl_ );
        } catch ( std::exception& ) {
            BOOST_THROW_EXCEPTION( serializer_error() << info( "serialize to xml_woarchive" ) );
        }
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

uint32_t
MSSimulatorMethod::chargeStateMin() const
{
    return impl_->charge_state_.first;
}

uint32_t
MSSimulatorMethod::chargeStateMax() const
{
    return impl_->charge_state_.second;
}

void
MSSimulatorMethod::setChargeStateMin( uint32_t value )
{
    impl_->charge_state_.first = value;
}

void
MSSimulatorMethod::setChargeStateMax( uint32_t value )
{
    impl_->charge_state_.second = value;
}

void
MSSimulatorMethod::setResolvingPower( double value )
{
    impl_->resolving_power_ = value;
}

double
MSSimulatorMethod::resolvingPower() const
{
    return impl_->resolving_power_;
}


bool
MSSimulatorMethod::isPositivePolarity() const
{
    return impl_->is_polarity_positive_;
}

void
MSSimulatorMethod::setIsPositivePolarity( bool value )
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
MSSimulatorMethod::isTof() const
{
    return impl_->is_tof_;
}

void
MSSimulatorMethod::setIsTof( bool value )
{
    impl_->is_tof_ = value;
}

double
MSSimulatorMethod::length() const
{
    return impl_->tof_length_;
}

void
MSSimulatorMethod::setLength( double value )
{
    impl_->tof_length_ = value;
}

double
MSSimulatorMethod::acceleratorVoltage() const
{
    return impl_->tof_accelerator_voltage_;
}

void
MSSimulatorMethod::setAcceleratorVoltage( double value )
{
    impl_->tof_accelerator_voltage_ = value;
}

double
MSSimulatorMethod::tDelay() const
{
    return impl_->tof_tDelay_;
}

void
MSSimulatorMethod::setTDelay( double value )
{
    impl_->tof_tDelay_ = value;
}

void
MSSimulatorMethod::setProtocol( int value )
{
    impl_->protocol_ = value;
}

int
MSSimulatorMethod::protocol() const
{
    return impl_->protocol_;
}

void
MSSimulatorMethod::setMode( int value )
{
    impl_->mode_ = value;
}

int
MSSimulatorMethod::mode() const
{
    return impl_->mode_;
}

void
MSSimulatorMethod::setAbundanceLowLimit( double limit )
{
    impl_->abundance_low_limit_ = limit;
}

double
MSSimulatorMethod::abundanceLowLimit() const
{
    return impl_->abundance_low_limit_;
}
