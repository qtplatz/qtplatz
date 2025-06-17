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

#include "scan_protocol.hpp"
#include <boost/json.hpp>
#include <stdexcept>
#include <string>
#include "accession.hpp"

namespace mzml {

    scan_protocol::scan_protocol() : ms_level_( 0 )
                                   , precursor_mz_( 0 )
                                   , collision_energy_( 0 )
                                   , polarity_( polarity_unknown )
    {
    }

    scan_protocol::scan_protocol( const scan_protocol& t ) : ms_level_( t.ms_level_ )
                                                           , precursor_mz_( t.precursor_mz_ )
                                                           , collision_energy_( t.collision_energy_ )
                                                           , polarity_( t.polarity_ )
    {
    }

    scan_protocol::scan_protocol( pugi::xml_node spectrum_node )
    {
        accession ac(spectrum_node );
        if ( auto level = ac.ms_level() )
            ms_level_ = *level;
        polarity_ = accession(spectrum_node).ion_polarity();

        if ( auto scan = spectrum_node.select_node( "./scanList[1]/scan" ) ) {
            if ( auto node =
                 spectrum_node.select_node( "./precursorList[1]/precursor/selectedIonList/selectedIon/cvParam[@accession='MS:1000744']" ) ) {
                precursor_mz_ = node.node().attribute( "value" ).as_double();
            }
            if ( auto node =
                 spectrum_node.select_node( "./precursorList[1]/precursor/activation/cvParam[@accession='MS:1000045']" ) ) {
                collision_energy_ = node.node().attribute( "value" ).as_double();
            }
        }
    }

    int
    scan_protocol::ms_level() const
    {
        return ms_level_;
    }

    double
    scan_protocol::precursor_mz() const
    {
        return precursor_mz_;
    }

    double
    scan_protocol::collision_energy() const
    {
        return collision_energy_;
    }

    ion_polarity_type
    scan_protocol::polarity() const
    {
        return polarity_;
    }

    void
    tag_invoke( const boost::json::value_from_tag, boost::json::value& jv, const scan_protocol& p )
    {
        jv = {
            {"ms_level", p.ms_level() }
            , {"precursor_mz", p.precursor_mz()}
            , {"collision_energy", p.collision_energy()}
            , {"polarity", p.polarity() == mzml::polarity_positive ? "positive"
               : p.polarity() == mzml::polarity_negative ? "negative"
               : "unknown"}
        };
    }

    void
    tag_invoke( const boost::json::value_from_tag, boost::json::value& jv, const scan_id& id )
    {
        jv = boost::json::value{
                {"index", std::get<0>(id) },
                {"id", std::get<1>(id) },
                {"scan_start_time", std::get<2>(id) }
            };
    }

    scan_identifier::scan_identifier()
    {
    }

    scan_id
    scan_identifier::operator()( const pugi::xml_node& spectrum_node ) const
    {
        if ( spectrum_node.name() != std::string( "spectrum" ) )
            throw std::invalid_argument( "not a spectrum node" );

        if ( auto scan = spectrum_node.select_node( "./scanList[1]/scan" ) ) {
            auto id = spectrum_node.attribute( "id" ).value();
            auto index = spectrum_node.attribute( "id" ).as_uint();
            double scan_start_time = scan.node().select_node( "cvParam[@accession='MS:1000016']" ).node().attribute("value").as_double();

            return { index, id, scan_start_time, scan_protocol( spectrum_node ) };
        }
        return {};
    }

} // namespace
