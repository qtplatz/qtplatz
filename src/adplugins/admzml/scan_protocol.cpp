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
#include "accession.hpp"
#include <adportable/debug.hpp>
#include <adportable/json/extract.hpp>
#include <adportable/json_helper.hpp>
#include <boost/json.hpp>
#include <boost/format.hpp>
#include <stdexcept>
#include <string>
#include <utility>

namespace mzml {

    scan_protocol::scan_protocol() : ms_level_( 0 )
                                   , precursor_mz_( 0 )
                                   , collision_energy_( 0 )
                                   , polarity_( polarity_unknown )
                                   , scan_window_lower_limit_(0)
                                   , scan_window_upper_limit_(0)
    {
    }

    scan_protocol::scan_protocol( const scan_protocol& t ) : ms_level_( t.ms_level_ )
                                                           , precursor_mz_( t.precursor_mz_ )
                                                           , collision_energy_( t.collision_energy_ )
                                                           , polarity_( t.polarity_ )
                                                           , scan_window_lower_limit_(t.scan_window_lower_limit_)
                                                           , scan_window_upper_limit_(t.scan_window_upper_limit_)
    {
    }

    scan_protocol::scan_protocol( pugi::xml_node spectrum_node ) : ms_level_( 0 )
                                                                 , precursor_mz_( 0 )
                                                                 , collision_energy_( 0 )
                                                                 , polarity_( polarity_unknown )
                                                                 , scan_window_lower_limit_(0)
                                                                 , scan_window_upper_limit_(0)
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
            if ( auto upper = scan.node().select_node( ".//scanWindow/cvParam[@accession='MS:1000500']" ) ) {
                scan_window_upper_limit_ = upper.node().attribute("value").as_double();
            }
            if ( auto lower = scan.node().select_node( ".//scanWindow/cvParam[@accession='MS:1000501']" ) ) {
                scan_window_lower_limit_ = lower.node().attribute("value").as_double();
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

    double
    scan_protocol::scan_window_lower_limit() const
    {
        return scan_window_lower_limit_;
    }

    double
    scan_protocol::scan_window_upper_limit() const
    {
        return scan_window_upper_limit_;
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
            auto index = spectrum_node.attribute( "index" ).as_uint();
            double scan_start_time = scan.node().select_node( "cvParam[@accession='MS:1000016']" ).node().attribute("value").as_double();

            return { index, id, scan_start_time, scan_protocol( spectrum_node ) };
        }
        return {};
    }

    std::ostream&
    operator<<(std::ostream& out, const scan_protocol& obj)
    {
        out << boost::format( "ms_level: %d, precursor_mz: %.2f, CE: %.1f, polarity: %s scan_mz={%.1f, %.1f}" )
            % obj.ms_level_
            % obj.precursor_mz_
            % obj.collision_energy_
            % (obj.polarity_ == polarity_negative ? "negative" : (obj.polarity_ == polarity_positive ? "positive" : "unknown" ) )
            % obj.scan_window_lower_limit_
            % obj.scan_window_upper_limit_;
        return out;
    }

} // namespace


namespace mzml {

    std::size_t
    protocol_key_hash::operator()(const scan_protocol_key_t& key) const
    {
        std::size_t seed = 0;
        hash_combine(seed, std::get<0>(key));
        hash_combine(seed, std::get<1>(key));
        hash_combine(seed, std::get<2>(key));
        hash_combine(seed, std::get<3>(key));
        hash_combine(seed, std::get<4>(key));
        hash_combine(seed, std::get<5>(key));
        return seed;
    }

    void
    tag_invoke( const boost::json::value_from_tag, boost::json::value& jv, const scan_protocol_key_t& t )
    {
        jv = boost::json::value{
            { "ms_level", std::get< 0 >( t ) }
            , { "polarity", std::get< 1 >( t ) }
            , { "precursor_mz", std::get< 2 >( t ) }
            , { "ce", std::get< 3 >( t ) }
            , { "scan_window", boost::json::value_from( std::make_tuple( std::get< 4 >( t ), std::get< 5 >( t ) ) ) }
        };
    }

    void
    tag_invoke( const boost::json::value_from_tag, boost::json::value& jv, const scan_protocol& p )
    {
        jv = {
            {"ms_level", p.ms_level() }
            , {"precursor_mz", p.precursor_mz()}
            , {"ce", p.collision_energy()}
            , {"polarity", p.polarity() == mzml::polarity_positive ? "positive"
               : p.polarity() == mzml::polarity_negative ? "negative"
               : "unknown"}
            , {"scan_window", { p.scan_window_lower_limit(), p.scan_window_upper_limit() } }
        };
    }

    scan_protocol
    tag_invoke( const boost::json::value_to_tag< scan_protocol >&, const boost::json::value& jv )
    {
        using namespace adportable::json;

        if ( jv.kind() == boost::json::kind::object ) {
            scan_protocol t;
            auto obj = jv.as_object();
            extract( obj, t.ms_level_, "ms_level" );
            extract( obj, t.precursor_mz_, "precursor_mz" );
            extract( obj, t.collision_energy_, "ce" );
            auto pol = obj.at( "polarity" ).as_string();
            t.polarity_ = ( pol == "positive" ) ? mzml::polarity_positive
                : ( pol == "negative" ) ? mzml::polarity_negative : mzml::polarity_unknown;
            t.scan_window_lower_limit_ = jv.at( "scan_window" ).as_array().at( 0 ).as_double();
            t.scan_window_lower_limit_ = jv.at( "scan_window" ).as_array().at( 1 ).as_double();
            return t;
        }
        return {};
    }

    void
    tag_invoke( const boost::json::value_from_tag, boost::json::value& jv, const scan_id& id )
    {
        jv = boost::json::value{
                {"index", std::get<0>(id) }
                , {"id", std::get<1>(id) }
                , {"scan_start_time", std::get<2>(id) }
                , { "scan_protocol", boost::json::value_from( std::get<3>(id) ) }
            };
    }


}
