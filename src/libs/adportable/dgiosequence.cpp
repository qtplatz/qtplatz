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

#include "dgiosequence.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/exception/all.hpp>
#include <iostream>

using namespace adportable::dg;

sample::sample() : runLength_( 60.0 )
                 , injVolume_( 1.0 )
{
}

sample::sample( const sample& t ) : runLength_( t.runLength_ )
                                  , injVolume_( t.injVolume_ )
                                  , sampleId_( t.sampleId_ )
                                  , description_( t.description_ )
                                  , methodId_( t.methodId_ )
{
}

sequence::sequence()
{
}

sequence::sequence( const sequence& t ) : samples_( t.samples_ )
                                        , replicates_( t.replicates_ )
{
}
            
bool
sequence::read_json( std::istream& is, sequence& t )
{
    t.samples().clear();
    boost::property_tree::ptree pt;

    try {
        boost::property_tree::read_json( is, pt );

        if ( auto value = pt.get_optional< size_t >( "sequence.replicates" ) )
            t.replicates() = value.get();

        if ( auto child = pt.get_child_optional( "samples" ) ) {
            for ( const auto& item: child.get() ) {
                sample s;
                read_json( item.second, s );
                t.samples().emplace_back( s );
            }
        }

    } catch ( std::exception& e ) {
        throw e;
    }
    return false;
}

bool
sequence::write_json( std::ostream& os, const sequence& t )
{
    boost::property_tree::ptree pt, pv;

    pt.put( "sequence.replicates", t.replicates() );

    for ( const auto& sample: t.samples() ) {
        boost::property_tree::ptree item;
        if ( write_json( item, sample ) )
            pv.push_back( std::make_pair( "", item ) );
    }
    pt.add_child( "sequence.samples", pv );

    boost::property_tree::write_json( os, pt );

    return true;
}

bool
sequence::read_json( const boost::property_tree::ptree& pt, sample& t )
{
    if ( auto value = pt.get_optional< double >( "runlength" ) )
        t.runLength_ = value.get();
    if ( auto value = pt.get_optional< double >( "injvolume" ) )
        t.injVolume_ = value.get();
    if ( auto value = pt.get_optional< std::string >( "sampleid" ) )
        t.sampleId_ = value.get();
    if ( auto value = pt.get_optional< std::string >( "description" ) )
        t.description_ = value.get();
    if ( auto value = pt.get_optional< std::string >( "methodid" ) )
        t.methodId_ = value.get();

    return true;
}

bool
sequence::write_json( boost::property_tree::ptree& pt, const sample& t )
{
    pt.put( "runlength",   t.runLength_ );
    pt.put( "injvolume",   t.injVolume_ );
    pt.put( "sampleid",    t.sampleId_ );
    pt.put( "description", t.description_ );
    pt.put( "methodid",    t.methodId_ );

    return true;
}

