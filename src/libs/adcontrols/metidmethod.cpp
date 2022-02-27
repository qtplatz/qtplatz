// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2022-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2022-2022 MS-Cheminformatics LLC
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

#include "metidmethod.hpp"
#include "msfinder.hpp"
#include <adportable/json/extract.hpp>

namespace boost { namespace serialization {  class access;  } }

namespace adcontrols {

    class MetIdMethod::impl {
    public:
        bool positiveMode_;
        std::vector< std::pair< bool, std::string > > adducts_;
        std::pair< uint32_t, uint32_t > chargeState_;
        idToleranceMethod toleranceMethod_;
        idFindAlgorithm findAlgorithm_;
        double tolerancePpm_;
        double toleranceDaltons_;
        impl() : positiveMode_( true )
               , adducts_{
                { true, "+[H]+"}, {true, "-H +[H]+"}, {true, "-H2O +[H]+"}
                , {false, "-[H]+" }, {false, "+H -[H]+"}, {false, "+[OH]-"}, {false, "+[Cl]-" } }
               , chargeState_{ 1, 1 }
               , toleranceMethod_( idToleranceDaltons )
               , findAlgorithm_( idFindClosest )
               , tolerancePpm_( 5 )
               , toleranceDaltons_( 0.010 ) { // Da (not mDa)
        }
        impl( const impl& t ) : positiveMode_( t.positiveMode_ )
                              , adducts_( t.adducts_ )
                              , chargeState_( t.chargeState_ )
                              , toleranceMethod_( t.toleranceMethod_ )
                              , findAlgorithm_( t.findAlgorithm_ )
                              , tolerancePpm_( t.tolerancePpm_ )
                              , toleranceDaltons_( t.toleranceDaltons_ ) {
        }
    };

}

using adcontrols::MetIdMethod;


MetIdMethod::~MetIdMethod()
{
}

MetIdMethod::MetIdMethod() : impl_( std::make_unique< MetIdMethod::impl >() )
{
}

MetIdMethod::MetIdMethod( const MetIdMethod& t )
    : impl_( std::make_unique< MetIdMethod::impl >( *t.impl_  ) )
{
}

MetIdMethod&
MetIdMethod::MetIdMethod::operator = ( const MetIdMethod& rhs )
{
    impl_ = std::make_unique< MetIdMethod::impl >( *rhs.impl_ );
    return *this;
}

bool
MetIdMethod::isPositiveMode() const
{
    return impl_->positiveMode_;
}

void
MetIdMethod::setPositiveMode( bool flag )
{
    impl_->positiveMode_ = flag;
}

MetIdMethod&
MetIdMethod::operator << ( std::pair< bool, std::string >&& d )
{
    impl_->adducts_.emplace_back( std::move( d ) );
    return *this;
}


void
MetIdMethod::setAdducts( const std::vector< std::pair< bool, std::string > >& t )
{
    impl_->adducts_ = t;
}

const std::vector< std::pair< bool, std::string > >&
MetIdMethod::adducts() const
{
    return impl_->adducts_;
}


std::pair< uint32_t, uint32_t>
MetIdMethod::chargeState() const
{
    return impl_->chargeState_;
}

void
MetIdMethod::chargeState( std::pair< uint32_t, uint32_t >&& charges )
{
    impl_->chargeState_ = charges;
}

adcontrols::idToleranceMethod
MetIdMethod::toleranceMethod() const
{
    return impl_->toleranceMethod_;
}

void
MetIdMethod::setToleranceMethod( idToleranceMethod id )
{
    impl_->toleranceMethod_ = id;
}

adcontrols::idFindAlgorithm
MetIdMethod::findAlgorithm() const
{
    return impl_->findAlgorithm_;
}

void
MetIdMethod::setFindAlgorithm( idFindAlgorithm algo )
{
    impl_->findAlgorithm_ = algo;
}

double
MetIdMethod::tolerance( idToleranceMethod id ) const
{
    return id == idTolerancePpm ? impl_->tolerancePpm_ : impl_->toleranceDaltons_;
}

double
MetIdMethod::tolerance() const
{
    return impl_->toleranceMethod_ == idTolerancePpm ? impl_->tolerancePpm_ : impl_->toleranceDaltons_;
}

void
MetIdMethod::setTolerance( idToleranceMethod id, double value )
{
    if ( id == idTolerancePpm )
        impl_->tolerancePpm_ = value;
    else
        impl_->toleranceDaltons_ = value;
}

namespace adcontrols {

    void
    tag_invoke( boost::json::value_from_tag, boost::json::value& jv, const MetIdMethod& t )
    {
        jv = boost::json::object{
            { "positiveMode", t.impl_->positiveMode_ }
            , { "adducts",            t.impl_->adducts_      }
            , { "chargeState",        t.impl_->chargeState_  }
            , { "idToleranceMethod",  int(t.impl_->toleranceMethod_) }
            , { "findAlgorithm",      int(t.impl_->findAlgorithm_) }
            , { "tolerancePpm",       t.impl_->tolerancePpm_ }
            , { "toleranceDaltons",   t.impl_->toleranceDaltons_ }
        };
    }

    MetIdMethod
    tag_invoke( boost::json::value_to_tag< MetIdMethod >&, const boost::json::value& jv )
    {
        MetIdMethod t;
        using namespace adportable::json;

        if ( jv.is_object() ) {
            auto obj = jv.as_object();
             extract( obj, t.impl_->positiveMode_    , "positiveMode" );
             extract( obj, t.impl_->adducts_         , "adducts" );
             extract( obj, t.impl_->chargeState_     , "chargeState" );
             int tmp;
             extract( obj, tmp                       , "idToleranceMethod" );
             t.impl_->toleranceMethod_ = static_cast< idToleranceMethod >( tmp );
             extract( obj, tmp                       , "findAlgorithm" );
             t.impl_->findAlgorithm_   = static_cast< idFindAlgorithm> ( tmp );
             extract( obj, t.impl_->tolerancePpm_    , "tolerancePpm" );
             extract( obj, t.impl_->toleranceDaltons_, "toleranceDaltons" );
        }
        return t;
    }

}
