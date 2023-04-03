/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "mspeak.hpp"
#include "chemicalformula.hpp"
#include "serializer.hpp"
#include <adportable/utf.hpp>

namespace adcontrols {

    class MSPeak::impl {
    public:
        ~impl() {}
        impl() : time_( 0 )
               , mass_( 0 )
               , mode_( 0 )
               , fcn_( 0 )
               , flength_( 0 )
               , spectrumIndex_( 0 )
               , time_width_( 0 )
               , mass_width_( 0 )
               , exit_delay_( 0 )
               , exact_mass_( 0 )
               , flags_(0) {
        }
        impl( const impl& t ) : time_( t.time_ )
                              , mass_( t.mass_ )
                              , mode_( t.mode_ )
                              , fcn_( t.fcn_ )
                              , flength_( t.flength_ )
                              , formula_( t.formula_ )
                              , description_( t.description_ )
                              , spectrumId_( t.spectrumId_ )
                              , spectrumIndex_( t.spectrumIndex_ )
                              , time_width_( t.time_width_ )
                              , mass_width_( t.mass_width_ )
                              , exit_delay_( t.exit_delay_ )
                              , exact_mass_( t.exact_mass_ )
                              , flags_( t.flags_ ) {
        }

        double time_;
        double mass_;
        int32_t mode_;  // corresponding to flight length
        int32_t fcn_;   // protocol id
        double flength_;
        std::string formula_;
        std::string description_;
        std::string spectrumId_;
        int32_t spectrumIndex_;
        double time_width_;
        double mass_width_;
        double exit_delay_;
        double exact_mass_;
        uint32_t flags_;

        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int version ) {
            using namespace boost::serialization;
            if ( version >= 3 ) {
                ar & BOOST_SERIALIZATION_NVP( time_ );
                ar & BOOST_SERIALIZATION_NVP( mass_ );
                ar & BOOST_SERIALIZATION_NVP( mode_ );
                ar & BOOST_SERIALIZATION_NVP( flength_ );
                ar & BOOST_SERIALIZATION_NVP( formula_ );
                ar & BOOST_SERIALIZATION_NVP( description_ );
                ar & BOOST_SERIALIZATION_NVP( spectrumId_ );
                ar & BOOST_SERIALIZATION_NVP( spectrumIndex_ );
                ar & BOOST_SERIALIZATION_NVP( time_width_ );
                ar & BOOST_SERIALIZATION_NVP( mass_width_ );
                ar & BOOST_SERIALIZATION_NVP( fcn_ );
                ar & BOOST_SERIALIZATION_NVP( exit_delay_ );
                ar & BOOST_SERIALIZATION_NVP( exact_mass_ );
                ar & BOOST_SERIALIZATION_NVP( flags_ );
            } else {
                std::wstring description;
                ar & BOOST_SERIALIZATION_NVP( time_ );
                ar & BOOST_SERIALIZATION_NVP( mass_ );
                ar & BOOST_SERIALIZATION_NVP( mode_ );
                ar & BOOST_SERIALIZATION_NVP( flength_ );
                ar & BOOST_SERIALIZATION_NVP( formula_ );
                ar & BOOST_SERIALIZATION_NVP( description );
                ar & BOOST_SERIALIZATION_NVP( spectrumId_ );
                ar & BOOST_SERIALIZATION_NVP( spectrumIndex_ );
                ar & BOOST_SERIALIZATION_NVP( time_width_ );
                ar & BOOST_SERIALIZATION_NVP( mass_width_ );
                if ( version >= 1 ) {
                    ar & BOOST_SERIALIZATION_NVP( fcn_ );
                    ar & BOOST_SERIALIZATION_NVP( exit_delay_ );
                    ar & BOOST_SERIALIZATION_NVP( exact_mass_ );
                }
                if ( version >= 2 )
                    ar & BOOST_SERIALIZATION_NVP( flags_ );
                //----
                if ( Archive::is_loading::value )
                    description_ = adportable::utf::to_utf8( description );
            }
        }
    };
}

using namespace adcontrols;

MSPeak::~MSPeak()
{
}

MSPeak::MSPeak() : impl_( std::make_unique< impl >() )
{
}

MSPeak::MSPeak( const MSPeak& t ) : impl_( std::make_unique< impl >( *t.impl_ ) )
{
}

const MSPeak&
MSPeak::operator = (const MSPeak& t )
{
    impl_ = std::make_unique< impl >(*t.impl_ );
    return *this;
}

MSPeak::MSPeak( double time
                , double mass
                , int32_t mode
                , double flength ) : impl_( std::make_unique< impl >() )
{
    impl_->mass_ = mass;
    impl_->mode_ = mode;
    impl_->fcn_ = 0;
    impl_->flength_ = flength;
}

MSPeak::MSPeak( const std::string& formula
                , double mass
                , double time
                , int32_t mode
                , int32_t spectrumIndex
                , double exact_mass ) : impl_( std::make_unique< impl >() )
{
    impl_->mass_ = mass;
    impl_->mode_ = mode;
    impl_->formula_ = formula;
    impl_->spectrumIndex_ = spectrumIndex;
    impl_->exact_mass_ = exact_mass;
}

double
MSPeak::time() const
{
    return impl_->time_;
}

double
MSPeak::mass() const
{
    return impl_->mass_;
}

int32_t
MSPeak::mode() const
{
    return impl_->mode_;
}

int32_t
MSPeak::fcn() const
{
    return impl_->fcn_;
}

void
MSPeak::fcn( int32_t v )
{
    impl_->fcn_ = v;
}

double
MSPeak::width( bool isTime ) const
{
    return isTime ? impl_->time_width_ : impl_->mass_width_;
}

void
MSPeak::width( double value, bool isTime )
{
    if ( isTime )
        impl_->time_width_ = value;
    else
        impl_->mass_width_ = value;
}

double
MSPeak::exit_delay() const
{
    return impl_->exit_delay_;
}

void
MSPeak::exit_delay( double v )
{
    impl_->exit_delay_ = v;
}

double
MSPeak::flight_length() const
{
    return impl_->flength_;
}

const std::string&
MSPeak::formula() const
{
    return impl_->formula_;
}

std::wstring
MSPeak::wdescription() const
{
    return adportable::utf::to_wstring( impl_->description_ );
}

std::string
MSPeak::description() const
{
    return impl_->description_;
}

const std::string&
MSPeak::spectrumId() const
{
    return impl_->spectrumId_;
}

int32_t
MSPeak::spectrumIndex() const
{
    return impl_->spectrumIndex_;
}

void
MSPeak::time( double v )
{
    impl_->time_ = v;
}

void
MSPeak::mass( double v )
{
    impl_->mass_ = v;
}

void
MSPeak::mode( int32_t v )
{
    impl_->mode_ = v;
}

void
MSPeak::flight_length( double v )
{
    impl_->flength_ = v;
}

void
MSPeak::formula( const std::string& v )
{
    impl_->formula_= v;
}

void
MSPeak::description( const std::wstring& v )
{
    impl_->description_ = adportable::utf::to_utf8( v );
}

void
MSPeak::description( const std::string& v )
{
    impl_->description_ = v;
}

void
MSPeak::spectrumId( const std::string& v )
{
    impl_->spectrumId_ = v;
}

void
MSPeak::spectrumIndex( int v )
{
    impl_->spectrumIndex_ = v;
}

double
MSPeak::exact_mass() const
{
    if ( !impl_->formula_.empty() && impl_->exact_mass_ < 0.7 )
        return ChemicalFormula().getMonoIsotopicMass( impl_->formula_ );
    return impl_->exact_mass_;
}

uint32_t
MSPeak::flags() const
{
    return impl_->flags_;
}

void
MSPeak::setFlags( uint32_t v )
{
    impl_->flags_ = v;
}

bool
MSPeak::isFlag( Flags f ) const
{
    return impl_->flags_ & f ? true : false;
}

namespace adcontrols {

    ////////// PORTABLE BINARY ARCHIVE //////////
    template<> void
    MSPeak::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        // saving
        impl_->serialize( ar, version );
    }

    template<> void
    MSPeak::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        // loading
        impl_->serialize( ar, version );
    }

    ///////// XML archive ////////
    template<> void
    MSPeak::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        // saving
        impl_->serialize( ar, version );
    }

    template<> void
    MSPeak::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        impl_->serialize( ar, version );
    }
}
