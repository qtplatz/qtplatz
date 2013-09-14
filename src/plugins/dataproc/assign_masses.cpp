/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include "assign_masses.hpp"
#include <adcontrols/msassignedmass.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/msreference.hpp>
#include <adportable/array_wrapper.hpp>
#include <cstring>

using namespace dataproc;

bool
assign_masses::operator()( adcontrols::MSAssignedMasses& assignedMasses
                           , const adcontrols::MassSpectrum& centroid
                           , const adcontrols::MSReferences& references
                           , int mode
                           , int fcn )
{
    using adportable::array_wrapper;
    using adcontrols::MSReferences;
    
    array_wrapper<const double> masses( centroid.getMassArray(), centroid.size() );
    array_wrapper<const double> intens( centroid.getIntensityArray(), centroid.size() );
    
    size_t idReference(0);
    for ( MSReferences::vector_type::const_iterator it = references.begin(); it != references.end(); ++it ) {
        
        double exactMass = it->exact_mass();
        array_wrapper<const double>::const_iterator lBound = std::lower_bound( masses.begin(), masses.end(), exactMass - tolerance_ );
        array_wrapper<const double>::const_iterator uBound = std::lower_bound( masses.begin(), masses.end(), exactMass + tolerance_ );
        
        if ( lBound != masses.end() ) {
            
            size_t lIdx = std::distance( masses.begin(), lBound );
            size_t uIdx = std::distance( masses.begin(), uBound );
            
            // find closest
            size_t cIdx = lIdx;
            for ( size_t i = lIdx + 1; i < uIdx; ++i ) {
                double d0 = std::abs( masses[ cIdx ] - exactMass );
                double d1 = std::abs( masses[ i ] - exactMass );
                if ( d1 < d0 )
                    cIdx = i;
            }
            
            // find highest
            array_wrapper<const double>::const_iterator hIt = std::max_element( intens.begin() + lIdx, intens.begin() + uIdx );
            if ( *hIt < threshold_ )
                continue;
            
            size_t idx = std::distance( intens.begin(), hIt );
            adcontrols::MSAssignedMass assigned( idReference
				                                 , fcn
                                                 , idx            // idMassSpectrum (index on centroid peak)
                                                 , it->display_formula()
                                                 , it->exact_mass()
                                                 , centroid.getTime( idx )
                                                 , masses[ idx ]
                                                 , it->enable()
                                                 , false          // flags
                                                 , mode );
            assignedMasses << assigned;
        }
        ++idReference;
    }
    return true;
    
}


