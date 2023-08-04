/**************************************************************************
** Copyright (C) 2022-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2022-2022 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "candidate.hpp"
#include "isopeak.hpp"
#include "simple_mass_spectrum.hpp"
#include <adcontrols/annotation.hpp>
#include <adcontrols/annotations.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/metidmethod.hpp>
#include <adportable/debug.hpp>
#include <adportable/json/extract.hpp>
#include <boost/format.hpp>
#include <vector>
#include <map>

using lipidid::simple_mass_spectrum;

namespace lipidid {

    class simple_mass_spectrum::impl {
    public:
        impl() {
        }
        impl( impl& t ) : data_( t.data_ )
                        , candidate_list_( t.candidate_list_ )
                        , isotope_match_result_( t.isotope_match_result_ ) {

        }
        std::vector< value_type > data_; // tof, mass, intensity, color, checked
        std::vector< std::vector< candidate > > candidate_list_;
        std::map< size_t, std::vector< isoPeak > > isotope_match_result_;
        std::unique_ptr< adcontrols::MetIdMethod > method_;
    };


}

simple_mass_spectrum::~simple_mass_spectrum()
{
}

simple_mass_spectrum::simple_mass_spectrum()
    : impl_( std::make_unique< impl >() )
{
}

simple_mass_spectrum::simple_mass_spectrum( const simple_mass_spectrum& t )
    : impl_( std::make_unique< impl >( *t.impl_ ) )
{
}

simple_mass_spectrum&
simple_mass_spectrum::operator = ( const simple_mass_spectrum& t )
{
    impl_ = std::make_unique< impl >( *t.impl_ );
    return *this;
}

void
simple_mass_spectrum::populate( const adcontrols::MassSpectrum& src
                                , std::function< bool( const adcontrols::mass_value_type& ) > pred )
{
    impl_->data_.clear();
    for ( size_t i = 0; i < src.size(); ++i ) {
        auto value = src.value( i );
        if ( pred( value ) ) {
            auto [tof,mass,intensity,color] = src.value( i );
            impl_->data_.emplace_back( tof, mass, intensity, color, false );
        }
    }
}

size_t
simple_mass_spectrum::size() const
{
    return impl_->data_.size();
}

simple_mass_spectrum::iterator
simple_mass_spectrum::begin()
{
    return impl_->data_.begin();
}

simple_mass_spectrum::iterator
simple_mass_spectrum::end()
{
    return impl_->data_.end();
}

simple_mass_spectrum::const_iterator
simple_mass_spectrum::begin() const
{
    return impl_->data_.begin();
}

simple_mass_spectrum::const_iterator
simple_mass_spectrum::end() const
{
    return impl_->data_.end();
}

void
simple_mass_spectrum::add_a_candidate( size_t idx, candidate&& t )
{
    if ( impl_->data_.size() != impl_->candidate_list_.size() ) {
        impl_->candidate_list_.clear();
        impl_->candidate_list_.resize( impl_->data_.size() );
    }
    assert( idx < impl_->candidate_list_.size() );
    impl_->candidate_list_[ idx ].emplace_back( t );
}

std::vector< lipidid::candidate >
simple_mass_spectrum::candidates( size_t idx ) const
{
    assert( idx < impl_->candidate_list_.size() );
    return impl_->candidate_list_.at( idx );
}

simple_mass_spectrum::value_type
simple_mass_spectrum::operator []( size_t idx ) const
{
    return impl_->data_.at( idx );
}

simple_mass_spectrum::value_type&
simple_mass_spectrum::at( size_t idx )
{
    return impl_->data_.at( idx );
}

std::vector< lipidid::isoPeak >
simple_mass_spectrum::find_cluster( size_t idx, const std::vector< std::pair< double, double > >& cluster ) const
{
    double observed_mass = mass_value_t::mass( impl_->data_[ idx ] );
    double abundance     = mass_value_t::intensity( impl_->data_[ idx ] );
    auto delta_m = cluster[ 0 ].first - observed_mass;

    std::vector< isoPeak > indices;
    std::transform( cluster.begin(), cluster.end(), std::back_inserter( indices ), [](const auto& a){ return a; } );

    size_t nmatch(0);//bool found( false );

    if ( auto base_index = find_peak( cluster[ 0 ].first - delta_m, 0.002 ) ) {
        for ( size_t i = 0; i < indices.size(); ++i ) {
            auto& isotope = indices[ i ].computed_isotope();
            if ( auto idx = find_peak( isotope.first - delta_m, 0.003 ) ) {
                ++nmatch;
                double mass_error = mass_value_t::mass( impl_->data_[ *idx ] ) - (isotope.first - delta_m);
                double ra_error   = mass_value_t::intensity( impl_->data_[ *idx ] ) / abundance;
                indices[ i ] = { true, *idx, mass_error, ra_error };
            }
        }
    }

    return indices; // impl_->isotope_match_result_[ idx ] = indices;
}

std::optional< size_t >
simple_mass_spectrum::find_peak( double mass, double tolerance ) const
{
    auto it = std::lower_bound( impl_->data_.begin(), impl_->data_.end(), mass - tolerance / 2.0
                                , [](const auto& a, double b){ return mass_value_t::mass(a) < b; });

    auto end = it;
    while ( mass_value_t::mass( *end ) < (mass + tolerance / 2.0) )
        ++end;

    if ( it != end ) {
        if ( end != impl_->data_.end() )
            ++end;
        auto pIt = std::min_element( it, end, [&](const auto& a, const auto& b){
            return std::abs(std::get<1>(a) - mass) < std::abs(std::get<1>(b) - mass); });
        return std::distance( impl_->data_.begin(), pIt );
    }

    return {};
}

std::optional< std::vector< lipidid::isoPeak > >
simple_mass_spectrum::cluster_match_result( size_t idx ) const
{
    auto it = impl_->isotope_match_result_.find( idx );
    if ( it != impl_->isotope_match_result_.end() )
        return it->second;
    return {};
}

simple_mass_spectrum&
simple_mass_spectrum::operator << ( std::pair< value_type, std::vector< candidate > >&& t )
{
    auto [ value, candidates ] = t;
    impl_->data_.emplace_back( value );
    impl_->candidate_list_.emplace_back( candidates );
    return *this;
}

// std::shared_ptr< adcontrols::MassSpectrum >
// simple_mass_spectrum::make_spectrum( const lipidid::simple_mass_spectrum& t ) const
// {
//     return {};
// }

std::shared_ptr< adcontrols::MassSpectrum >
simple_mass_spectrum::make_spectrum( const candidate& candidate, std::shared_ptr< const adcontrols::MassSpectrum > refms ) const
{
    // ADDEBUG() << "--------- simple_mass_spectrum::make_spectrum ---------";
    auto ms = std::make_shared< adcontrols::MassSpectrum >();
    ms->clone( *refms, false );
    try {
        std::vector< double > masses, intensities;
        std::vector< uint8_t > colors;

        size_t ioffs(0);
        try {
            for ( const auto& ipk: candidate.isotope() ) {
                auto [ found, index, mass_error, ra_error ] = ipk.matched_isotope_;
                // ADDEBUG() << ipk.matched_isotope_;
                if ( found && ( index >= impl_->data_.size() ) ) {
                    const auto& value = (*this)[ index ];
                    masses.emplace_back( mass_value_t::mass( value ) );
                    intensities.emplace_back( mass_value_t::intensity( value ) );
                    if ( ioffs == 0 ) {
                        colors.emplace_back( 7 ); // crimson
                        ms->get_annotations()
                            << adcontrols::annotation( candidate.formula() + candidate.adduct()
                                                       , masses.back()
                                                       , intensities.back()
                                                       , masses.size() - 1 // index
                                                       , int( intensities.back() )
                                                       , adcontrols::annotation::dataFormula
                                                       , adcontrols::annotation::flag_targeting );
                    } else {
                        double offs = ipk.computed_isotope_.first - candidate.isotope()[ 0 ].computed_isotope_.first;
                        colors.emplace_back( 7 ); // crimson
                        ms->get_annotations()
                            << adcontrols::annotation( "+" + std::to_string( int( offs + 0.7 ) )
                                                       , masses.back()
                                                       , intensities.back()
                                                       , masses.size() - 1 // index
                                                       , int( intensities.back() )
                                                       , adcontrols::annotation::dataText
                                                       , adcontrols::annotation::flag_targeting );
                    }
                }
                (void)mass_error;
                (void)ra_error;
                ++ioffs;
            }
        } catch ( std::out_of_range& ex ) {
            ADDEBUG() << "## exception: " << ex.what();
        }
        ms->setMassArray( std::move( masses ) );
        ms->setIntensityArray( std::move( intensities ) );
        ms->setColorArray( std::move( colors ) );
    } catch ( std::out_of_range& ex ) {
        ADDEBUG() << "## exception: " << ex.what();
    }
    return ms;
}

std::unique_ptr< adcontrols::MetIdMethod >&
simple_mass_spectrum::method() const
{
    return impl_->method_;
}

void
simple_mass_spectrum::set_method( std::unique_ptr< adcontrols::MetIdMethod >&& t )
{
    impl_->method_ = std::move( t );
}

namespace lipidid {

    void
    tag_invoke( boost::json::value_from_tag, boost::json::value& jv, const simple_mass_spectrum& ms )
    {
        using lipidid::mass_value_t;
        boost::json::array ja;
        for ( size_t i = 0; i < ms.size(); ++i ) {
            const auto& value = ms[ i ];
            boost::json::object jobj = {
                { "idx", i }
                , { "mass",       mass_value_t::mass( value ) }
                , { "intensity",  mass_value_t::intensity( value ) }
                , { "color",      mass_value_t::color( value ) }
                , { "checked",    mass_value_t::checked( value ) }
                , { "candidates", ms.candidates( i ) }
            };
            ja.emplace_back( jobj );
        }
        jv = boost::json::object{{ "peaks", ja }};
    }

    simple_mass_spectrum
    tag_invoke( boost::json::value_to_tag< simple_mass_spectrum >&, const boost::json::value& jv )
    {
        simple_mass_spectrum t;
        if ( auto peaks = jv.as_object().if_contains( "peaks" ) ) {
            const auto& ja = peaks->as_array();
            for ( const auto& value: ja ) {
                simple_mass_spectrum::value_type v;
                std::vector< candidate > candidates;

                const auto& obj = value.as_object();
                adportable::json::extract( obj, std::get< 1 >( v ), "mass"       );
                adportable::json::extract( obj, std::get< 2 >( v ), "intensity"  );
                adportable::json::extract( obj, std::get< 3 >( v ), "color"      );
                adportable::json::extract( obj, std::get< 4 >( v ), "checked"      );
                adportable::json::extract( obj, candidates,         "candidates" );
                t << std::make_pair( v, candidates );
            }
        }
        return t;
    }
}
