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

#include "targetingmethod.hpp"
#include "serializer.hpp"
#include "moltable.hpp"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>

namespace adcontrols {

    class TargetingMethod::impl {
    public:
        impl( idTarget id ) : idTarget_( id )
            , toleranceMethod_( idToleranceDaltons )
            , tolerancePpm_( 10.0 )
            , toleranceDaltons_( 0.010 ) // 10mDa
            , chargeStateMin_( 1 )
            , chargeStateMax_( 3 )
            , isLowMassLimitEnabled_( false ) // auto
            , isHighMassLimitEnabled_( false )
            , lowMassLimit_( 1 )
            , highMassLimit_( 1000 )
            , tolerance_( 10.0 ) { // deplicated
            // reference,
            // http://fiehnlab.ucdavis.edu/staff/kind/Metabolomics/MS-Adduct-Calculator/
            for ( auto adduct : { "H", "Na", "NH4", "K", "CH3CN+H" "CH3CN+Na", "CH3OH+H", "(CH3)2SO+H", "C3H8O+H", "C3H8O+Na" } )
                pos_adducts_.push_back( std::make_pair( false, adduct ) );


            for ( auto adduct : { "-H", "-H2O-H", "Na-H2", "Cl", "K-H2", "COOH-H" } )
                neg_adducts_.push_back( std::make_pair( false, adduct ) );
        }
        
        idTarget idTarget_;
        idToleranceMethod toleranceMethod_;
        double tolerancePpm_;
        double toleranceDaltons_;
        uint32_t chargeStateMin_;
        uint32_t chargeStateMax_;
        bool isLowMassLimitEnabled_;
        bool isHighMassLimitEnabled_;
        double lowMassLimit_;
        double highMassLimit_;
        double tolerance_;

        std::vector< std::pair< bool, std::string > > pos_adducts_; // if start with '-' means lose instead of add
        std::vector< std::pair< bool, std::string > > neg_adducts_;

        std::vector< formula_type > formulae_;
        std::vector< peptide_type > peptides_;
        moltable molecules_;
        //----------
        
        friend class boost::serialization::access;
        template<class Archive> void serialize(Archive& ar, const unsigned int version ) {
            using namespace boost::serialization;

            bool is_use_resolving_power; // workaround

            if ( version <= 1 ) {
                std::vector< std::pair< std::wstring, bool > > adducts;
                std::vector< std::pair< std::wstring, bool > > formulae;
                bool isPositive;

                ar & BOOST_SERIALIZATION_NVP( isPositive );
                ar & BOOST_SERIALIZATION_NVP( is_use_resolving_power );
                ar & BOOST_SERIALIZATION_NVP( tolerancePpm_ );
                ar & BOOST_SERIALIZATION_NVP( toleranceDaltons_ );
                ar & BOOST_SERIALIZATION_NVP( chargeStateMin_ );
                ar & BOOST_SERIALIZATION_NVP( chargeStateMax_ );
                ar & BOOST_SERIALIZATION_NVP( isLowMassLimitEnabled_ );
                ar & BOOST_SERIALIZATION_NVP( isHighMassLimitEnabled_ );
                ar & BOOST_SERIALIZATION_NVP( lowMassLimit_ );
                ar & BOOST_SERIALIZATION_NVP( highMassLimit_ );
                ar & BOOST_SERIALIZATION_NVP( tolerance_ );
                ar & BOOST_SERIALIZATION_NVP( formulae );
                ar & BOOST_SERIALIZATION_NVP( adducts ); // pos
                ar & BOOST_SERIALIZATION_NVP( adducts ); // neg
            } else {
                ar & BOOST_SERIALIZATION_NVP( idTarget_ )
                    ;
                if ( version < 4 )
                    ar & BOOST_SERIALIZATION_NVP( is_use_resolving_power );
                else if ( version >= 4 )
                    ar & BOOST_SERIALIZATION_NVP( toleranceMethod_ );
                ar & BOOST_SERIALIZATION_NVP( tolerancePpm_ )
                    & BOOST_SERIALIZATION_NVP( toleranceDaltons_ )
                    & BOOST_SERIALIZATION_NVP( chargeStateMin_ )
                    & BOOST_SERIALIZATION_NVP( chargeStateMax_ )
                    & BOOST_SERIALIZATION_NVP( isLowMassLimitEnabled_ )
                    & BOOST_SERIALIZATION_NVP( isHighMassLimitEnabled_ )
                    & BOOST_SERIALIZATION_NVP( lowMassLimit_ )
                    & BOOST_SERIALIZATION_NVP( highMassLimit_ )
                    & BOOST_SERIALIZATION_NVP( tolerance_ )
                    & BOOST_SERIALIZATION_NVP( pos_adducts_ )
                    & BOOST_SERIALIZATION_NVP( neg_adducts_ )
                    ;
                if ( version < 3 ) {
                    std::vector< std::pair< bool, std::string > > formulae;
                    std::vector< std::pair< bool, std::pair< std::string, std::string > > > peptides;
                    ar & BOOST_SERIALIZATION_NVP( formulae )
                        & BOOST_SERIALIZATION_NVP( peptides )
                        ;
                } else {
                    ar & BOOST_SERIALIZATION_NVP( formulae_ )
                        & BOOST_SERIALIZATION_NVP( peptides_ )
                        ;
                }
            }
        }
    };
    ////////// PORTABLE BINARY ARCHIVE //////////
    template<> void
    TargetingMethod::serialize( portable_binary_oarchive& ar, const unsigned int )
    {
        ar & *impl_;
    }

    template<> void
    TargetingMethod::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        impl_->serialize( ar, version );
    }

    ///////// XML archive ////////
    template<> void
    TargetingMethod::serialize( boost::archive::xml_woarchive& ar, const unsigned int )
    {
        ar & boost::serialization::make_nvp("impl", *impl_);
    }

    template<> void
    TargetingMethod::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        impl_->serialize( ar, version );
    }
}


using namespace adcontrols;

TargetingMethod::TargetingMethod( idTarget id ) : impl_( new impl( id ) )
                                                , idTarget_( id )
                                                , toleranceMethod_( idToleranceDaltons )
                                                , tolerancePpm_( 10.0 )
                                                , toleranceDaltons_( 0.010 ) // 10mDa
                                                , chargeStateMin_( 1 )
                                                , chargeStateMax_( 3 )
                                                , isLowMassLimitEnabled_( false ) // auto
                                                , isHighMassLimitEnabled_( false )
                                                , lowMassLimit_( 1 )
                                                , highMassLimit_( 1000 )
                                                , tolerance_( 10.0 ) // deplicated
{
    // reference, 
    // http://fiehnlab.ucdavis.edu/staff/kind/Metabolomics/MS-Adduct-Calculator/
    
    pos_adducts_.push_back( std::make_pair( false, "H" ) );
    pos_adducts_.push_back( std::make_pair( false, "Na" ) );
    pos_adducts_.push_back( std::make_pair( false, "NH4" ) );
    pos_adducts_.push_back( std::make_pair( false, "K" ) );
    pos_adducts_.push_back( std::make_pair( false, "CH3CN+H" ) );
    pos_adducts_.push_back( std::make_pair( false, "CH3CN+Na" ) );
    pos_adducts_.push_back( std::make_pair( false, "CH3OH+H" ) );
    pos_adducts_.push_back( std::make_pair( false, "(CH3)2SO+H" ) ); // DMSO+H
    pos_adducts_.push_back( std::make_pair( false, "C3H8O+H" ) );    // IPA+H
    pos_adducts_.push_back( std::make_pair( false, "C3H8O+Na" ) );   // IPA+Na

    neg_adducts_.push_back( std::make_pair( false, "-H" ) );
    neg_adducts_.push_back( std::make_pair( false, "-H2O-H" ) );
    neg_adducts_.push_back( std::make_pair( false, "Na-H2" ) );
    neg_adducts_.push_back( std::make_pair( false, "Cl" ) );
    neg_adducts_.push_back( std::make_pair( false, "K-H2" ) );
    neg_adducts_.push_back( std::make_pair( false, "COOH-H" ) );
}

TargetingMethod::TargetingMethod( const TargetingMethod& t )
{
    operator = ( t );
}

TargetingMethod&
TargetingMethod::operator = ( const TargetingMethod& rhs )
{
    idTarget_ = rhs.idTarget_;

    toleranceMethod_        = rhs.toleranceMethod_;
    tolerancePpm_           = rhs.tolerancePpm_;
    toleranceDaltons_       = rhs.toleranceDaltons_;
    chargeStateMin_         = rhs.chargeStateMin_;
    chargeStateMax_         = rhs.chargeStateMax_;
    isLowMassLimitEnabled_  = rhs.isLowMassLimitEnabled_;
    isHighMassLimitEnabled_ = rhs.isHighMassLimitEnabled_;
    lowMassLimit_           = rhs.lowMassLimit_;
    highMassLimit_          = rhs.highMassLimit_;
    tolerance_              = rhs.tolerance_;

    formulae_               = rhs.formulae_;
    peptides_               = rhs.peptides_;
    pos_adducts_            = rhs.pos_adducts_;
    neg_adducts_            = rhs.neg_adducts_;

	return *this;
}

void
TargetingMethod::targetId( TargetingMethod::idTarget target )
{
    idTarget_ = target;
}

TargetingMethod::idTarget
TargetingMethod::targetId() const
{
    return idTarget_;
}

std::vector< std::pair< bool, std::string > >&
TargetingMethod::adducts( bool positive )
{
    return positive ? pos_adducts_ : neg_adducts_;
}

const std::vector< std::pair< bool, std::string > >&
TargetingMethod::adducts( bool positive ) const
{
    return positive ? pos_adducts_ : neg_adducts_;
}

std::pair< uint32_t, uint32_t >
TargetingMethod::chargeState() const
{
    return std::pair< uint32_t, uint32_t >( chargeStateMin_, chargeStateMax_ );
}

void
TargetingMethod::chargeState( uint32_t min, uint32_t max )
{
    chargeStateMin_ = min;
    chargeStateMax_ = max;
}

std::vector< TargetingMethod::formula_type >&
TargetingMethod::formulae()
{
    return formulae_;
}

const std::vector< TargetingMethod::formula_type >&
TargetingMethod::formulae() const
{
    return formulae_;
}

std::vector< TargetingMethod::peptide_type >&
TargetingMethod::peptides()
{
    return peptides_;
}

const std::vector< TargetingMethod::peptide_type >&
TargetingMethod::peptides() const
{
    return peptides_;
}

idToleranceMethod
TargetingMethod::toleranceMethod() const
{
    return toleranceMethod_;
}

void
TargetingMethod::setToleranceMethod( idToleranceMethod value )
{
    toleranceMethod_ = value;
}

double
TargetingMethod::tolerance( idToleranceMethod id ) const
{
    return id == idTolerancePpm ? tolerancePpm_ : toleranceDaltons_;
}

void
TargetingMethod::setTolerance( idToleranceMethod id, double value )
{
    if ( id == idTolerancePpm )
        tolerancePpm_ = value;
    else
        toleranceDaltons_ = value;
}


std::pair< bool, bool >
TargetingMethod::isMassLimitsEnabled() const
{
    return std::pair<bool, bool>( isLowMassLimitEnabled_, isHighMassLimitEnabled_ );
}

void
TargetingMethod::isLowMassLimitEnabled( bool value )
{
    isLowMassLimitEnabled_ = value;
}

void
TargetingMethod::isHighMassLimitEnabled( bool value )
{
    isHighMassLimitEnabled_ = value;
}
        
double
TargetingMethod::lowMassLimit() const
{
    return lowMassLimit_;
}

void
TargetingMethod::lowMassLimit( double value )
{
    lowMassLimit_ = value;
}

double
TargetingMethod::highMassLimit() const
{
    return highMassLimit_;
}

void
TargetingMethod::highMassLimit( double value )
{
    highMassLimit_ = value;
}

#if 0
double
TargetingMethod::tolerance() const
{
    return tolerance_;
}

void
TargetingMethod::tolerance( double value )
{
    tolerance_ = value;
}
#endif
