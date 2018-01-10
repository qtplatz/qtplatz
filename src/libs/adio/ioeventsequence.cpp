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

#include "ioconfig.hpp"
#include "ioeventsequence.hpp"
#include <adportable/debug.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/exception/all.hpp>
#include <boost/format.hpp>
#include <array>
#include <iostream>

using namespace adio;

ioEventSequence::ioEventSequence() : methodTime_( 10.0 )
{
}

ioEventSequence::ioEventSequence( const ioEventSequence& t ) : methodTime_( t.methodTime_ )
                                                             , sequence_( t.sequence_ )
{
}

double
ioEventSequence::methodTime() const
{
    return methodTime_;
}

void
ioEventSequence::setMethodTime( double t )
{
    methodTime_ = t;
}


bool
ioEventSequence::read_json( std::istream& json, ioEventSequence& m )
{
    boost::property_tree::ptree pt;

    m.sequence().clear();

    try {
        boost::property_tree::read_json( json, pt );

        if ( auto mTime = pt.get_optional< double >( "methodTime" ) )
            m.setMethodTime( mTime.get() );

        if ( auto child = pt.get_child_optional( "sequence" ) ) {

            for ( const auto& row: child.get() ) {
                if ( auto time = row.second.get_optional< double >( "time" ) ) {

                    row_type t( time.get(), std::array< ioEvent, 4 >() );

                    if ( auto events = row.second.get_child_optional( "ioEvents" ) ) {
                        size_t column( 0 );
                        for ( const auto& event: events.get() ) {
                            if ( auto id = event.second.get_optional< std::string >( "id" ) ) {
                                if ( auto action = event.second.get_optional< std::string >( "action" ) )
                                    t.second[ column ] = { id.get(), action.get() };
                            }
                            column++;
                        }
                    }
                    m.sequence().emplace_back( t );
                }
            }
        }
        return true;
    } catch ( std::exception& e ) {
        std::cout << boost::diagnostic_information( e ) << std::endl;
    }
    return false;
}

bool
ioEventSequence::write_json( std::ostream& json, const ioEventSequence& m, bool pritty_print )
{
    boost::property_tree::ptree j_seq;

    for ( const auto& line: m.sequence() ) {
        
        boost::property_tree::ptree j_item, j_events;
        j_item.put( "time", line.first );

        boost::property_tree::ptree a;
        for ( const auto& action: line.second ) {
            a.put( "id", action.id() );
            a.put( "action", action.action() );
            j_events.push_back( std::make_pair( "", a ) ); // <-- std::array< ioEvent, 4 >
        }
        j_item.add_child( "ioEvents", j_events );
        j_seq.push_back( std::make_pair( "", j_item ) );
    }

    boost::property_tree::ptree pt;
    pt.put( "methodTime", m.methodTime() );
    pt.add_child( "sequence", j_seq );

    boost::property_tree::write_json( json, pt, pritty_print );
    
    return true;
}

