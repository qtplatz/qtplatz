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
                            , findAlgorithm_( idFindLargest )
                            , tolerancePpm_( 10.0 )
                            , toleranceDaltons_( 0.010 ) // 10mDa
                            , chargeStateMin_( 1 )
                            , chargeStateMax_( 1 )
                            , isLowMassLimitEnabled_( false ) // auto
                            , isHighMassLimitEnabled_( false )
                            , lowMassLimit_( 1 )
                            , highMassLimit_( 1000 )
                            , tolerance_( 0 ) /* not in use */ {
            // reference,
            // http://fiehnlab.ucdavis.edu/staff/kind/Metabolomics/MS-Adduct-Calculator/

            // "(CH3)2SO+H" // DMSO+H
            // "C3H8O+H"    // IPA+H
            // "C3H8O+Na"   // IPA+Na
            for ( auto adduct : { "H", "Na", "NH4", "K", "CH3CN+H" "CH3CN+Na", "CH3OH+H", "(CH3)2SO+H", "C3H8O+H", "C3H8O+Na" } )
                pos_adducts_.push_back( std::make_pair( false, adduct ) );

            for ( auto adduct : { "-H", "-H2O-H", "Na-H2", "Cl", "K-H2", "COOH-H" } )
                neg_adducts_.push_back( std::make_pair( false, adduct ) );
        }
        
        idTarget idTarget_;
        idToleranceMethod toleranceMethod_;
        idFindAlgorithm findAlgorithm_;
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
            } else if ( version <= 4 ) {
                ar & BOOST_SERIALIZATION_NVP( idTarget_ );
                if ( version <= 3 )
                    ar & BOOST_SERIALIZATION_NVP( is_use_resolving_power );
                else if ( version == 4 )
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

                if ( version <= 3 ) {
                    
                    std::vector< std::pair< bool, std::string > > formulae;
                    std::vector< std::pair< bool, std::pair< std::string, std::string > > > peptides;
                    ar & BOOST_SERIALIZATION_NVP( formulae ) & BOOST_SERIALIZATION_NVP( peptides );

                } else {
                    typedef std::pair< std::string, std::pair< bool, std::wstring > > formula_type;
                    typedef std::pair< bool, std::pair< std::string, std::string > > peptide_type;
                    
                    std::vector< formula_type > formulae;
                    std::vector< peptide_type > peptides;
                    ar & BOOST_SERIALIZATION_NVP( formulae ) & BOOST_SERIALIZATION_NVP( peptides );

                    for ( auto& f: formulae ) {
                        moltable::value_type mol;
                        mol.formula() = f.first;
                        mol.enable() = f.second.first;
                        mol.description() = f.second.second;
                        molecules_ << mol;
                    }
                }
            } else if ( version >= 5 ) {
                ar & BOOST_SERIALIZATION_NVP( idTarget_ );
                ar & BOOST_SERIALIZATION_NVP( findAlgorithm_ );
                ar & BOOST_SERIALIZATION_NVP( toleranceMethod_ );
                ar & BOOST_SERIALIZATION_NVP( tolerancePpm_ );
                ar & BOOST_SERIALIZATION_NVP( toleranceDaltons_ );
                ar & BOOST_SERIALIZATION_NVP( chargeStateMin_ );
                ar & BOOST_SERIALIZATION_NVP( chargeStateMax_ );
                ar & BOOST_SERIALIZATION_NVP( isLowMassLimitEnabled_ );
                ar & BOOST_SERIALIZATION_NVP( isHighMassLimitEnabled_ );
                ar & BOOST_SERIALIZATION_NVP( lowMassLimit_ );
                ar & BOOST_SERIALIZATION_NVP( highMassLimit_ );
                ar & BOOST_SERIALIZATION_NVP( tolerance_ );
                ar & BOOST_SERIALIZATION_NVP( pos_adducts_ );
                ar & BOOST_SERIALIZATION_NVP( neg_adducts_ );
                ar & BOOST_SERIALIZATION_NVP( molecules_ );
            }
        }
    };
    ////////// PORTABLE BINARY ARCHIVE //////////
    template<> void
    TargetingMethod::serialize( portable_binary_oarchive& ar, const unsigned int )
    {
        ar & boost::serialization::make_nvp( "impl", *impl_ );
    }

    template<> void
    TargetingMethod::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        if ( version <= 4 )        
            impl_->serialize( ar, version );
        else
            ar & boost::serialization::make_nvp("impl", *impl_);        
    }

    ///////// XML archive ////////
    template<> void
    TargetingMethod::serialize( boost::archive::xml_woarchive& ar, const unsigned int )
    {
        ar & boost::serialization::make_nvp( "impl", *impl_ );
    }

    template<> void
    TargetingMethod::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        if ( version <= 4 )
            impl_->serialize( ar, version );
        else
            ar & boost::serialization::make_nvp("impl", *impl_);
    }
}

BOOST_CLASS_VERSION( adcontrols::TargetingMethod::impl, 5 )

using namespace adcontrols;

TargetingMethod::TargetingMethod( idTarget id ) : impl_( new impl( id ) )
{
}

TargetingMethod::TargetingMethod( const TargetingMethod& t ) : impl_( new impl( *t.impl_ ) )
{
}

TargetingMethod&
TargetingMethod::operator = ( const TargetingMethod& rhs )
{
    if ( impl_ != rhs.impl_ ) {
        delete impl_;
        impl_ = new impl( *rhs.impl_ );
    }
	return *this;
}

void
TargetingMethod::targetId( TargetingMethod::idTarget target )
{
    impl_->idTarget_ = target;
}

TargetingMethod::idTarget
TargetingMethod::targetId() const
{
    return impl_->idTarget_;
}

std::vector< std::pair< bool, std::string > >&
TargetingMethod::adducts( bool positive )
{
    return positive ? impl_->pos_adducts_ : impl_->neg_adducts_;
}

const std::vector< std::pair< bool, std::string > >&
TargetingMethod::adducts( bool positive ) const
{
    return positive ? impl_->pos_adducts_ : impl_->neg_adducts_;
}

std::pair< uint32_t, uint32_t >
TargetingMethod::chargeState() const
{
    return std::pair< uint32_t, uint32_t >( impl_->chargeStateMin_, impl_->chargeStateMax_ );
}

void
TargetingMethod::chargeState( uint32_t min, uint32_t max )
{
    impl_->chargeStateMin_ = min;
    impl_->chargeStateMax_ = max;
}

idToleranceMethod
TargetingMethod::toleranceMethod() const
{
    return impl_->toleranceMethod_;
}

void
TargetingMethod::setToleranceMethod( idToleranceMethod value )
{
    impl_->toleranceMethod_ = value;
}

idFindAlgorithm
TargetingMethod::findAlgorithm() const
{
    return impl_->findAlgorithm_;
}

void
TargetingMethod::setFindAlgorithm( idFindAlgorithm algo )
{
    impl_->findAlgorithm_ = algo;
}

double
TargetingMethod::tolerance( idToleranceMethod id ) const
{
    return id == idTolerancePpm ? impl_->tolerancePpm_ : impl_->toleranceDaltons_;
}

void
TargetingMethod::setTolerance( idToleranceMethod id, double value )
{
    if ( id == idTolerancePpm )
        impl_->tolerancePpm_ = value;
    else
        impl_->toleranceDaltons_ = value;
}


std::pair< bool, bool >
TargetingMethod::isMassLimitsEnabled() const
{
    return std::pair<bool, bool>( impl_->isLowMassLimitEnabled_, impl_->isHighMassLimitEnabled_ );
}

void
TargetingMethod::isLowMassLimitEnabled( bool value )
{
    impl_->isLowMassLimitEnabled_ = value;
}

void
TargetingMethod::isHighMassLimitEnabled( bool value )
{
    impl_->isHighMassLimitEnabled_ = value;
}
        
double
TargetingMethod::lowMassLimit() const
{
    return impl_->lowMassLimit_;
}

void
TargetingMethod::lowMassLimit( double value )
{
    impl_->lowMassLimit_ = value;
}

double
TargetingMethod::highMassLimit() const
{
    return impl_->highMassLimit_;
}

void
TargetingMethod::highMassLimit( double value )
{
    impl_->highMassLimit_ = value;
}

const moltable&
TargetingMethod::molecules() const
{
    return impl_->molecules_;
}

moltable&
TargetingMethod::molecules()
{
    return impl_->molecules_;
}

void
TargetingMethod::setMolecules( const moltable& t )
{
    impl_->molecules_ = t;
}

