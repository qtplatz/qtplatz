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

#include "simple_mass_spectrum.hpp"
#include "isopeak.hpp"
#include "candidate.hpp"
#include <adcontrols/massspectrum.hpp>
#include <adportable/debug.hpp>
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
        std::vector< std::tuple< double, double, double, int > > data_; // tof, mass, intensity, color
        std::vector< std::vector< candidate > > candidate_list_;
        std::map< size_t, std::vector< isoPeak > > isotope_match_result_;
    };


    void tag_invoke( boost::json::value_from_tag, boost::json::value& jv, const candidate& t )
    {
        // auto dataItems = boost::json::array();
        // for ( const auto& list: t.dataItems ) {
        //     boost::json::object obj;
        //     for ( const auto& d: list ) {
        //         if ( std::find_if( exclude.begin(), exclude.end(), [&](const auto& a){ return d.first == a; }) == exclude.end() )
        //             obj[ d.first ] = d.second;
        //     }
        //     dataItems.emplace_back( std::move( obj ) );
        // }

        //     , { "dataItems",  dataItems } };
    }
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

void
simple_mass_spectrum::populate( const adcontrols::MassSpectrum& src
                                , std::function< bool( const value_type& ) > pred )
{
    impl_->data_.clear();
    for ( size_t i = 0; i < src.size(); ++i ) {
        auto value = src.value( i );
        if ( pred( value ) ) {
            impl_->data_.emplace_back( value );
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

std::vector< lipidid::isoPeak >
simple_mass_spectrum::find_cluster( size_t idx, const std::vector< std::pair< double, double > >& cluster ) const
{
    double observed_mass = mass_value_t::mass( impl_->data_[ idx ] );
    double abundance     = mass_value_t::intensity( impl_->data_[ idx ] );
    auto delta_m = cluster[ 0 ].first - observed_mass;

    std::vector< isoPeak > indices( cluster.size() );

    bool found( false );

    if ( auto base_index = find_peak( cluster[ 0 ].first - delta_m, 0.002 ) ) {
        for ( size_t i = 0; i < cluster.size(); ++i ) {
            if ( auto it = find_peak( cluster[ i ].first - delta_m, 0.003 ) ) {
                double dm  = mass_value_t::mass( impl_->data_[ *it ] ) - (cluster[ i ].first - delta_m);
                double ra  = mass_value_t::intensity( impl_->data_[ *it ] ) / abundance;
                indices[ i ] = isoPeak{*it, ra / cluster[ i ].second, dm };
                if ( i > 0 )
                    found = true;
            }
        }
    }

    if ( found )
        return indices; // impl_->isotope_match_result_[ idx ] = indices;

    return {};
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

namespace lipidid {

    void
    tag_invoke( boost::json::value_from_tag, boost::json::value&, const simple_mass_spectrum& ms )
    {
        using lipidid::mass_value_t;
        for ( size_t i = 0; i < ms.size(); ++i ) {
            boost::json::array ja;
            for ( size_t i = 0; i < ms.size(); ++i ) {
                const auto& value = ms[ i ];
                boost::json::object jobj = {
                    { "idx", i }
                    , { "mass", mass_value_t::mass( value ) }
                    , { "intensity", mass_value_t::intensity( value ) }
                    , { "color", mass_value_t::color( value ) }
                    , { "candidates", ms.candidates( i ) }
                };
                ja.emplace_back( jobj );
            }
        }
    }

}
