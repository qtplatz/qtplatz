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
#include "make_reference_spectrum.hpp"
#include "mol.hpp"
#include "simple_mass_spectrum.hpp"
#include "isocluster.hpp"
#include "isopeak.hpp"
#include <adcontrols/annotation.hpp>
#include <adcontrols/annotations.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/metidmethod.hpp>
#include <adfs/get_column_values.hpp>
#include <adportable/debug.hpp>
#include <adwidgets/progressinterface.hpp>

using lipidid::make_reference_spectrum;

make_reference_spectrum::make_reference_spectrum() // std::map< std::string
// , std::vector< std::shared_ptr< lipidid::mol > > >& mols ) // : mols_( mols )
{
}

std::shared_ptr< adcontrols::MassSpectrum >
make_reference_spectrum::operator()(const adcontrols::MassSpectrum& ms
                                    , const simple_mass_spectrum& simple_ms )
{
    auto refMs = std::make_shared< adcontrols::MassSpectrum >();
    std::vector< double > masses, intensities;
    std::vector< uint8_t > colors;
    refMs->clone( ms, false );
    int cid(0);

    for ( size_t idx = 0; idx < simple_ms.size(); ++idx ) {
        auto [ tof, mass, intensity, color, checked ] = simple_ms[ idx ];

        auto candidates = simple_ms.candidates( idx );

        if ( ! candidates.empty() ) {
            const auto& candidate = candidates.at( 0 );
            auto cluster = isoCluster::compute( candidate.formula(), candidate.adduct() );
            refMs->get_annotations()
                << adcontrols::annotation( candidate.formula() + " " + candidate.adduct()
                                           , cluster.at(0).first // mass
                                           , cluster.at(0).second * intensity
                                           , masses.size() // index
                                           , cluster.at(0).second * intensity
                                           , adcontrols::annotation::dataFormula
                                           , adcontrols::annotation::flag_targeting );
            for ( const auto& ipk: cluster ) {
                masses.emplace_back( ipk.first );
                intensities.emplace_back( ipk.second * intensity );
                colors.emplace_back( cid & 1 ? 0 : 6 ); // blue, indigo
            }
            ++cid;
        }
    }
    refMs->setMassArray( std::move( masses ) );
    refMs->setIntensityArray( std::move( intensities ) );
    refMs->setColorArray( std::move( colors ) );
    return refMs;
}
