/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "jcb2009_helper.hpp"
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/peaks.hpp>
#include <adcontrols/peak.hpp>
#include <adportable/debug.hpp>
#include <adportable/json_helper.hpp>
#include <adportfolio/folium.hpp>
#include <boost/json.hpp>

using namespace adprocessor;
using namespace adprocessor::jcb2009_helper;

void
printer::print( const portfolio::Folium& folium )
{
    if ( auto chro = portfolio::get< adcontrols::ChromatogramPtr >( folium ) ) {
        ADDEBUG() << folium.name() << "\tpeaks.size: " << chro->peaks().size();

        auto jv = adportable::json_helper::parse( chro->generatorProperty() );
        if ( jv.is_object() ) {
            if ( auto gen = jv.as_object().if_contains( "generator" ) ) {
                if ( auto value = gen->as_object().if_contains( "extract_by_peak_info" ) ) {
                    auto mv = adportable::json_helper::find( *value, "pkinfo.mass" );
                    ADDEBUG() << "extract_by_peak_info: mass = " << mv.as_double();
                    } else if ( auto value = gen->as_object().if_contains( "extract_by_mols" ) ) {
                    ADDEBUG() << "extract_by_mols: " << adportable::json_helper::find( *value, "moltable" );
                } else if ( auto value = gen->as_object().if_contains( "extract_by_axis_range" ) ) {
                    ADDEBUG() << "extract_by_axis_range: " << *value;
                }
            }
        }
    }

}

adcontrols::Peaks
find_peaks::get( const portfolio::Folium& folium )
{
    if ( auto chro = portfolio::get< adcontrols::ChromatogramPtr >( folium ) ) {
        return chro->peaks();
    }
    return {};
}

std::tuple< double, double, double >
find_peaks::tR( const adcontrols::Peak& pk, double w )
{
    return { pk.peakTime()
                  , pk.peakTime() - std::abs(pk.startTime() - pk.peakTime()) / w
                  , pk.peakTime() + std::abs(pk.peakTime() - pk.endTime()) / w };
}
