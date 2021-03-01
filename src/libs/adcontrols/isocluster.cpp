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

#include "isocluster.hpp"
#include "chemicalformula.hpp"
#include "element.hpp"
#include "isotopes.hpp"
#include "lapfinder.hpp"
#include "massspectrometer.hpp"
#include "massspectrum.hpp"
#include "molecule.hpp"
#include "scanlaw.hpp"
#include "tableofelement.hpp"
#include <adportable/debug.hpp>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <numeric>
#include <sstream>

using namespace adcontrols;

isoCluster::isoCluster() : threshold_abundance_( 1.0e-12 )
                         , resolving_power_( 100000 )
{
}

isoCluster::isoCluster( double abundance_threshold
                        , double resolving_power ) : threshold_abundance_( abundance_threshold )
                                                   , resolving_power_( resolving_power )
{
}

bool
isoCluster::operator()( mol::molecule& mol, int charge ) const
{
    mol.cluster().clear();
    mol << mol::isotope( 0.0, 1.0 ); // trigger calculation

    // loop for each element e.g. 'C', 'H', 'N', ...
    for ( auto& element : mol.elements() ) {

        // loop for element count e.g. C6
        for ( int k = 0; k < element.count(); ++k ) {

            std::vector< mol::isotope > cluster;

            for ( auto& p: mol.cluster() ) {

                for ( auto& i: element.isotopes() ) {

                    mol::isotope mi( p.mass + i.mass, p.abundance * i.abundance );
                    if ( mi.abundance > threshold_abundance_ ) {
                        // make an array of order of mass
                        auto it = std::lower_bound( cluster.begin(), cluster.end(), mi.mass
                                                    , []( const mol::isotope& mi, const double& mass ){
                                                        return mi.mass < mass;
                                                    });
                        if ( it == cluster.end() || !merge( *it, mi ) )
                            cluster.emplace( it, mi );
                    }
                }
            }

            mol.cluster() = std::move( cluster );
        }
    }

    if ( charge > 0 ) {
        std::for_each( mol.cluster_begin(), mol.cluster_end()
                       , [&]( mol::isotope& i ){ i.mass = ( i.mass - TableOfElement::instance()->electronMass() * charge ) / charge; } );
        // std::transform( mol.cluster_begin(), mol.cluster_end(), mol.cluster_begin()
        //                 , [&]( auto& pk ){ return mol::isotope( ( pk.mass - TableOfElement::instance()->electronMass() * charge ) / charge, pk.abundance ); });
    } else if ( charge < 0 ) {
        std::for_each( mol.cluster_begin(), mol.cluster_end()
                       , [&]( mol::isotope& i ){ i.mass = ( i.mass + TableOfElement::instance()->electronMass() * (-charge) ) / (-charge); } );
        // std::transform( mol.cluster_begin(), mol.cluster_end(), mol.cluster_begin()
        //                 , [&]( auto& pk ){ return mol::isotope( ( pk.mass + TableOfElement::instance()->electronMass() * (-charge) ) / (-charge), pk.abundance ); });
    }
    return true;
}

boost::optional< mol::molecule >
isoCluster::compute( std::string&& formula, std::string&& adduct )
{
    return boost::none;
}

bool
isoCluster::merge( mol::isotope& it, const mol::isotope& mi ) const
{
    if ( mi.abundance <= threshold_abundance_ )
        return true; // throw it away

    if ( std::abs( it.mass - mi.mass ) < threshold_daltons_ ) {
        it.abundance += mi.abundance;

        // weighting average for mass -- this may affected when other independent molecule is co-exist
        double m = ( it.mass * it.abundance + mi.mass * mi.abundance ) / ( it.abundance + mi.abundance );

        it.mass = m;
        return true;
    }
    return false;
}
