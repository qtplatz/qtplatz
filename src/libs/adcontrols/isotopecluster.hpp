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

#pragma once

#include "adcontrols_global.h"

#include <string>
#include <vector>

namespace adcontrols {

	namespace mol { struct molecule; struct isotope; }

    class MassSpectrum;
    class ADCONTROLSSHARED_EXPORT isotopeCluster;

    class isotopeCluster {
    public:
        isotopeCluster();
        isotopeCluster( double abundance_threshold, double resolving_power );

        struct ADCONTROLSSHARED_EXPORT isopeak {
            double mass;
            double abundance; // keep toe value
            int index;
            isopeak( double m = 0, double a = 0, int i = (-1) ) : mass( m ), abundance( a ), index(i) {}
            isopeak( const isopeak& t ) : mass(t.mass), abundance(t.abundance), index(t.index) {}
        };

        [[deprecated]] bool operator()( adcontrols::MassSpectrum& ms, const std::string& formula
                                        , double relative_abundance = 1.0, double resolving_power = 1000000.0 );

        [[deprecated]] bool operator()( adcontrols::MassSpectrum& ms
                                        , const std::vector< std::pair<std::string, double > >& formula_abundance
                                        , double resolving_power = 1000000.0 );

        bool operator()( adcontrols::MassSpectrum&
                         , const std::vector< std::tuple< std::string, double, int > >& formula_mass_charge
                         , double resolving_power );

        /*
         * This function returns relative abundance that base peak abundance as 1.0
         */
        std::vector< isopeak > operator()( const std::vector< std::pair< std::string, char > >& formulae, int charge, int index = (-1) );

        bool operator()( mol::molecule&, int charge ) const;

        double threshold_daltons() const;
        void setThreshold_daltons( double d );

        static std::vector< std::string > formulae( const std::string& formula );

    private:
        /*
         * This function returns relative abundance that keeps table-of-element value
         */
        bool operator()( std::vector< isopeak >&, const std::string& formula, double relative_abundance = 1.0, int index = ( -1 ) ) const; // historical

        static void merge_peaks( std::vector<isopeak>&, double resolving_power );

        bool merge( mol::isotope&, const mol::isotope& ) const;
        double threshold_daltons_;
        double threshold_abundance_;
        double resolving_power_;
    };

}
