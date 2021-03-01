// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC
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
#include <memory>

namespace adcontrols {

	namespace mol { struct molecule; struct isotope; }

    class MassSpectrometer;
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

        bool operator()( adcontrols::MassSpectrum&
                         , const std::vector< std::tuple< std::string, double, int > >& formula_mass_charge
                         , double resolving_power );

        // for multi-turn
        // bool operator()( adcontrols::MassSpectrum&
        //                  , const std::vector< std::tuple< std::string, double, int > >& formula_mass_charge
        //                  , double resolving_power
        //                  , std::shared_ptr< const adcontrols::MassSpectrometer >
        //                  , int lap );

        static  std::shared_ptr< adcontrols::MassSpectrum >
        toMassSpectrum( const std::vector< adcontrols::mol::molecule >&
                        , std::shared_ptr< const adcontrols::MassSpectrum >
                        , std::shared_ptr< const adcontrols::MassSpectrometer >
                        , int lap );
        /*
         * This function returns relative abundance that base peak abundance as 1.0
         */
        std::vector< isopeak > operator()( const std::vector< std::pair< std::string, char > >& formulae, int charge, int index = (-1) );

        // bool compute( mol::molecule&, int charge ) const;

        double threshold_daltons() const;
        void setThreshold_daltons( double d );

        static std::vector< std::string > formulae( const std::string& formula );

    private:
        static  std::shared_ptr< adcontrols::MassSpectrum >
        __toMTSpectrum( const std::vector< adcontrols::mol::molecule >&
                        , std::shared_ptr< const adcontrols::MassSpectrum >
                        , std::shared_ptr< const adcontrols::MassSpectrometer >
                        , int lap );
        static  std::shared_ptr< adcontrols::MassSpectrum >
        __toMassSpectrum( const std::vector< adcontrols::mol::molecule >&
                          , std::shared_ptr< const adcontrols::MassSpectrum >
                          , std::shared_ptr< const adcontrols::MassSpectrometer >
                          , int mode );

        static void merge_peaks( std::vector<isopeak>&, double resolving_power );

        bool merge( mol::isotope&, const mol::isotope& ) const;
        double threshold_daltons_;
        double threshold_abundance_;
        double resolving_power_;
    };

}
