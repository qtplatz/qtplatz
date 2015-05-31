/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <tuple>

namespace quan {

    class QuanChromatogram;

    /**
     * QuanTarget lists all possible masses and formulae represent for a user specified compunds (formula)
     */

    class QuanTarget : public std::enable_shared_from_this< QuanTarget > {
        
        QuanTarget( const QuanTarget& ) = delete;
        QuanTarget& operator = ( const QuanTarget& ) = delete;

    public:
        QuanTarget( const std::string& formula
                    , bool positive_polarity = true
                    , const std::pair<int, int>& charge_range = { 1, 1 }
                    , const std::vector< std::string >& adducts_losses = std::vector< std::string >() );

        const std::string& formula() const; // core formula

        typedef std::tuple< std::string, double, int > value_type; // 'formula(+|-)adducts', mass, charge

        const std::vector< value_type >& formulae() const;

        static inline std::string formula( const value_type& t ) { return std::get< 0 >( t ); }
        static inline double exactMass( const value_type& t ) { return std::get< 1 >( t ); }
        static inline int charge( const value_type& t ) { return std::get< 2 >( t ); }

        typedef std::tuple< std::string, double, int, double, double > computed_candidate_value;

        static inline std::string formula( const computed_candidate_value& t ) { return std::get< 0 >( t ); }
        static inline double exactMass( const computed_candidate_value& t ) { return std::get< 1 >( t ); }
        static inline int charge( const computed_candidate_value& t ) { return std::get< 2 >( t ); }
        static inline double matchedMass( const computed_candidate_value& t ) { return std::get< 3 >( t ); }
        static inline double matchedWidth( const computed_candidate_value& t ) { return std::get< 4 >( t ); }

        void compute_candidate_masses( double mspeak_width, double tolerance
                                       , std::vector< std::tuple<std::string, double, int, double, double> >& ); // formula, exact, charge, matched, width

    private:
        std::string formula_;
        bool positive_polarity_;
        std::pair< int, int > charge_range_; // range for charge state
        std::vector< std::string > adducts_; // adducts | losses

        std::vector< std::tuple< std::string, double, int > > formulae_;                    // <formula,exactMass,charge> := list of candidates
    };

}

