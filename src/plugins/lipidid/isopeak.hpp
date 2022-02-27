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

namespace lipidid {

    struct isoPeak {
        std::pair< double, double > computed_isotope_; // mass, r.a.
        std::tuple< bool, size_t, double, double > matched_isotope_; // found, peak_index, mass error, ra error

        const std::pair< double, double >& computed_isotope() const { return computed_isotope_; };
        bool found() const { return std::get< 0 >( matched_isotope_ ); }
        size_t peak_index() const { return std::get< 1 >( matched_isotope_ ); }
        double mass_error() const { return std::get< 2 >( matched_isotope_ ); }
        double ra_error() const { return std::get< 3 >( matched_isotope_ ); }
        isoPeak( const std::pair< double, double >& i
                 , bool found = false
                 , size_t peak_index = 0
                 , double mass_error = -1
                 , double ra_error = 0 );

        isoPeak( const isoPeak& );
        isoPeak& operator = ( std::tuple< bool, size_t, double, double >&& );
    };

    void tag_invoke( boost::json::value_from_tag, boost::json::value&, const isoPeak& );
    isoPeak tag_invoke( boost::json::value_to_tag< isoPeak >&, const boost::json::value& );
}
