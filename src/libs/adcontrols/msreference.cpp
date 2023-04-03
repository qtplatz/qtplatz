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

#include "constants.hpp"
#include "msreference.hpp"
#include "chemicalformula.hpp"
#include "serializer.hpp"
#include <adportable/utf.hpp>
#include <sstream>

namespace adcontrols {

    class MSReference::impl {
    public:
        impl() : enable_( true )
               , exactMass_(0)
               , polarity_( polarity_positive )
               , chargeCount_( 1 ) {
        }

        impl( const impl& t ) : enable_( t.enable_ )
                              , exactMass_( t.exactMass_ )
                              , polarity_( t.polarity_ )
                              , chargeCount_( t.chargeCount_ )
                              , formula_( t.formula_ )
                              , adduct_or_loss_( t.adduct_or_loss_ )
                              , description_( t.description_ ) {
        }

        bool enable_;
        double exactMass_;
        ion_polarity polarity_;
        uint32_t chargeCount_;
        std::string formula_;
        std::string adduct_or_loss_;
        std::string description_;

        // exclude from serialize
        std::string display_formula_;

		void compute_mass();

        friend class boost::serialization::access;
        template<class Archive>
        void serialize( Archive& ar, const unsigned int version ) {
            (void)version;
            using namespace boost::serialization;
            if ( version < 2 ) {
                std::wstring formula, adduct_or_loss, description;
                bool polarityPositive( true );
                ar & BOOST_SERIALIZATION_NVP(enable_);
                ar & BOOST_SERIALIZATION_NVP(exactMass_);
                ar & boost::serialization::make_nvp( "polarityPositive", polarityPositive );
                ar & BOOST_SERIALIZATION_NVP(chargeCount_);
                ar & boost::serialization::make_nvp( "formula",          formula );
                ar & boost::serialization::make_nvp( "adduct_or_loss",   adduct_or_loss );
                ar & boost::serialization::make_nvp( "description",      description );
                if ( Archive::is_loading::value ) {
                    polarity_       = polarityPositive ? polarity_positive : polarity_negative;
                    formula_        = adportable::utf::to_utf8( formula );
                    adduct_or_loss_ = adportable::utf::to_utf8( adduct_or_loss );
                    description_    = adportable::utf::to_utf8( description );
                }
            } else {
                ar & BOOST_SERIALIZATION_NVP(enable_);
                ar & BOOST_SERIALIZATION_NVP(exactMass_);
                ar & BOOST_SERIALIZATION_NVP(polarity_);
                ar & BOOST_SERIALIZATION_NVP(chargeCount_);
                ar & BOOST_SERIALIZATION_NVP(formula_);
                ar & BOOST_SERIALIZATION_NVP(adduct_or_loss_);
                ar & BOOST_SERIALIZATION_NVP(description_);
            }
        }

    };

}

namespace adcontrols {

    ////////// PORTABLE BINARY ARCHIVE //////////
    template<> void
    MSReference::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        impl_->serialize( ar, version );
    }

    template<> void
    MSReference::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        impl_->serialize( ar, version );
    }

    ///////// XML archive ////////
    template<> void
    MSReference::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        impl_->serialize( ar, version );
    }

    template<> void
    MSReference::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        impl_->serialize( ar, version );
    }
}


using namespace adcontrols;

MSReference::~MSReference()
{
}

MSReference::MSReference() : impl_( new impl )
{
}

MSReference::MSReference( const MSReference& t ) : impl_( new impl( *t.impl_ ) )
{
}

MSReference::MSReference( const std::string& formula
                          , ion_polarity polarity
                          , const std::string& adduct_or_loss
                          , bool enable
                          , double exactMass
                          , uint32_t charge
                          , const std::string& description ) : impl_( new impl )
{
    impl_->enable_ = enable;
    impl_->exactMass_ = exactMass;
    impl_->polarity_ = polarity;
    impl_->chargeCount_ = charge;
    impl_->formula_ = formula;
    impl_->adduct_or_loss_ = adduct_or_loss;
    impl_->description_ = description;

    if ( exactMass <= std::numeric_limits<double>::epsilon() )
        impl_->compute_mass();
}


MSReference&
MSReference::operator = ( const MSReference& t )
{
    impl_.reset( new impl( *t.impl_ ) );
    return *this;
}

void
MSReference::impl::compute_mass()
{
    ChemicalFormula formula;
    exactMass_ = formula.getMonoIsotopicMass( formula_ );

    if ( ! adduct_or_loss_.empty() ) {
        auto adductlist = adcontrols::ChemicalFormula::split( adduct_or_loss_ );
        exactMass_ += adcontrols::ChemicalFormula().getMonoIsotopicMass( adductlist ).first;
    }
}

bool
MSReference::enable() const
{
    return impl_->enable_;
}

bool
MSReference::operator < ( const MSReference& t ) const
{
    return impl_->exactMass_ < t.impl_->exactMass_;
}

double
MSReference::exact_mass() const
{
    return impl_->exactMass_;
}

bool
MSReference::polarityPositive() const
{
    return impl_->polarity_ == polarity_positive;
}

// const wchar_t *
// MSReference::wformula() const
// {
//     return impl_->formula_.c_str();
// }

// const wchar_t *
// MSReference::wdisplay_formula() const
// {
//     impl_->display_formula_ = impl_->formula_ + impl_->adduct_or_loss_;
//     return impl_->display_formula_.c_str();
// }

// const wchar_t *
// MSReference::wadduct_or_loss() const
// {
//     return impl_->adduct_or_loss_.c_str();
// }

// const wchar_t *
// MSReference::wdescription() const
// {
//     return impl_->description_.c_str();
// }

std::string
MSReference::formula() const
{
    return impl_->formula_;
}

std::string
MSReference::display_formula() const
{
    impl_->display_formula_ = impl_->formula_ + impl_->adduct_or_loss_;
    return impl_->display_formula_;
}

std::string
MSReference::adduct_or_loss() const
{
    return impl_->adduct_or_loss_;
}

std::string
MSReference::description() const
{
    return impl_->description_;
}

void
MSReference::enable( bool value )
{
    impl_->enable_ = value;
}

void
MSReference::exact_mass( double value )
{
    impl_->exactMass_ = value;
}

void
MSReference::charge_count( uint32_t v )
{
	impl_->chargeCount_ = v;
}

uint32_t
MSReference::charge_count() const
{
	return impl_->chargeCount_;
}

void
MSReference::polarityPositive( bool value )
{
    // impl_->polarityPositive_ = value;
    impl_->polarity_ = value ? polarity_positive : polarity_negative;
}

void
MSReference::set_polarity( ion_polarity t )
{
    impl_->polarity_ = t;
}

ion_polarity
MSReference::polarity() const
{
    return impl_->polarity_;
}


void
MSReference::formula( const std::string& value )
{
    impl_->formula_ = value;
}

void
MSReference::adduct_or_loss( const std::string& value )
{
    impl_->adduct_or_loss_ = value;
}

void
MSReference::description( const std::string& value )
{
    impl_->description_ = value;
}
