// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include <compiler/disable_unused_parameter.h>
#include "isotopecluster.hpp"
#include "tableofelement.hpp"
#include "molecule.hpp"
#include "isotopes.hpp"
#include "element.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>

using namespace adcontrols;

isotopeCluster::isotopeCluster() : threshold_daltons_( 1.0e-7 )
                                 , threshold_abundance_( 1.0e-15 ) 
{
}

bool
isotopeCluster::operator()( mol::molecule& mol ) const
{
    mol.cluster.clear();
    mol.cluster.push_back( mol::isotope( 0.0, 1.0 ) ); // trigger calculation
    
    for ( auto& element: mol.elements ) {

        for ( int k = 0; k < element.count(); ++k ) {

            std::vector< mol::isotope > cluster;
            
            for ( auto& p: mol.cluster ) {

                for ( auto& i: element.isotopes() ) {

                    mol::isotope mi( p.mass + i.mass, p.abundance * i.abundance );

                    // make an array of order of mass
                    auto it = std::lower_bound( cluster.begin(), cluster.end(), mi.mass, []( const mol::isotope& mi, const double& mass ){
                            return mi.mass < mass; });

                    if ( it == cluster.end() || !marge( *it, mi ) )
                        cluster.insert( it, mi );
                }
            }
            
            mol.cluster = cluster;
        }
    }
    return true;
}

bool
isotopeCluster::marge( mol::isotope& it, const mol::isotope& mi ) const
{
    if ( mi.abundance <= threshold_abundance_ )
        return true; // throw it away

    if ( ( it.mass - mi.mass ) < threshold_daltons_ ) {
        it.abundance += mi.abundance;
        // don't change mass (or take average?)
        return true;
    }
    return false;
}

