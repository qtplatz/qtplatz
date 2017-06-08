// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC
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

#include "dgprotocols.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/exception/all.hpp>
#include <boost/format.hpp>
#include <iostream>

static void
print( const boost::property_tree::ptree& pt )
{
    using boost::property_tree::ptree;

    ptree::const_iterator end = pt.end();
    for (ptree::const_iterator it = pt.begin(); it != end; ++it) {
        std::cout << it->first << ": " << it->second.get_value<std::string>() << std::endl;
        print(it->second);
    }    
}


// using namespace adportable::dg;

namespace dg {
    
    template<> bool
    protocols< protocol<> >::read_json( std::istream& json, protocols< protocol<> >& protocols )
    {
        protocols.protocols_.clear();
        
        boost::property_tree::ptree pt;
        
        try {
            boost::property_tree::read_json( json, pt );

            protocols.interval_ = std::stod( pt.get_child( "protocols.interval" ).data() ) * 1.0e-6; // us -> seconds
                
            for ( const auto& v : pt.get_child( "protocols.protocol" ) ) {

                protocol<delay_pulse_count> data;

                int index = std::stoi( v.second.get_child( "index" ).data() );
                int replicates = std::stoi( v.second.get_child( "replicates" ).data() );

                data.setReplicates( replicates );

                size_t ch(0);
                for ( const auto& pulse: v.second.get_child( "pulses" ) ) {
                    double delay, width;
                    if ( ch < protocol<>::size ) {
                        delay = std::stod( pulse.second.get_child( "delay" ).data() );
                        width = std::stod( pulse.second.get_child( "width" ).data() );
                        data[ int(ch) ] = std::make_pair( delay * 1.0e-6, width * 1.0e-6 );
                    }
                    ++ch;
                    // std::cout << "----- ch : " << ch << " delay: " << delay << " width: " << width << std::endl;
                }
                protocols.protocols_.emplace_back( data );
                // std::cout << "----- protocol : " << protocols_.size() << std::endl;
            }

            return true;
        
        } catch ( std::exception& e ) {
            // std::cout << boost::diagnostic_information( e );
            throw e;
        }
        return false;
    }

    /////////////////////
        
    template<> bool
    protocols< protocol<> >::write_json( std::ostream& o, const protocols< protocol<> >& protocols )
    {
        boost::property_tree::ptree pt;
    
        pt.put( "protocols.interval", protocols.interval_ * 1.0e6 ); // seconds --> us
    
        boost::property_tree::ptree pv;
    
        int protocolIndex( 0 );
    
        for ( const auto& protocol: protocols.protocols_ ) {
        
            boost::property_tree::ptree xproto;
        
            xproto.put( "index", protocolIndex++ );
            xproto.put( "replicates", protocol.replicates() );
        
            boost::property_tree::ptree xpulses;
        
            for ( const auto& pulse: protocol.pulses() ) {
                boost::property_tree::ptree xpulse;

                xpulse.put( "delay", ( boost::format( "%.3lf" ) % ( pulse.first * 1.0e6 ) ).str() );
                xpulse.put( "width", ( boost::format( "%.3lf" ) % ( pulse.second * 1.0e6 ) ).str() );

                xpulses.push_back( std::make_pair( "", xpulse ) );
            }
        
            xproto.add_child( "pulses", xpulses );

            pv.push_back( std::make_pair( "", xproto ) );
        }
    
        pt.add_child( "protocols.protocol", pv );

        boost::property_tree::write_json( o, pt );
    
        return true;
    }

}

