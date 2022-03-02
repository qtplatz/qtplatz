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

#pragma once
#include <boost/json/value_from.hpp>
#include <boost/json/value_to.hpp>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

namespace adcontrols {
    class MassSpectrum;
    class MetIdMethod;
    typedef std::tuple< double, double, double, int > mass_value_type;
}

namespace lipidid {

    struct isoPeak;
    class candidate;

    struct mass_value_t {
        // std::tuple< double (tof), double (mass), double (intensity), int8_t(color) >
        template< typename value_type > static double time_of_flight( const value_type& v ){ return std::get< 0 >( v ); };
        template< typename value_type > static double mass( const value_type& v ){ return std::get< 1 >( v ); };
        template< typename value_type > static double intensity( const value_type& v ){ return std::get< 2 >( v ); };
        template< typename value_type > static int color( const value_type& v ){ return std::get< 3 >( v ); };
        template< typename value_type > static bool checked( const value_type& v ){ return std::get< 4 >( v ); };
        template< typename value_type > static bool& checked( value_type& v ){ return std::get< 4 >( v ); };
    };

    class simple_mass_spectrum {
    public:
        typedef std::tuple< double, double, double, int, bool > value_type; // tof, mass, intensity, color, checked
        typedef std::vector< value_type >::iterator iterator;
        typedef std::vector< value_type >::const_iterator const_iterator;
        ~simple_mass_spectrum();
        simple_mass_spectrum();
        simple_mass_spectrum( const simple_mass_spectrum& t );
        simple_mass_spectrum& operator = ( const simple_mass_spectrum& t );
        void populate( const adcontrols::MassSpectrum&, std::function< bool( const adcontrols::mass_value_type& ) > pred = [](auto){ return true; } );
        size_t size() const;
        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;
        value_type operator []( size_t idx ) const;
        value_type& at( size_t idx );

        void add_a_candidate( size_t idx, candidate&& );
        std::vector< candidate > candidates( size_t idx ) const;

        std::vector< isoPeak > find_cluster( size_t idx, const std::vector< std::pair< double, double > >& ) const;
        std::optional< std::vector< isoPeak > > cluster_match_result( size_t idx ) const;

        simple_mass_spectrum& operator << ( std::pair< value_type, std::vector< candidate > >&& );

        std::shared_ptr< adcontrols::MassSpectrum > make_spectrum( const lipidid::simple_mass_spectrum& ) const;
        std::unique_ptr< adcontrols::MetIdMethod >& method() const;
        void set_method( std::unique_ptr< adcontrols::MetIdMethod >&& );

    private:
        class impl;
        std::unique_ptr< impl > impl_;
        std::optional< size_t > find_peak( double mass, double tolerance ) const;
    };

    void tag_invoke( boost::json::value_from_tag, boost::json::value&, const simple_mass_spectrum& );
    simple_mass_spectrum tag_invoke( boost::json::value_to_tag< simple_mass_spectrum >&, const boost::json::value& );

}
