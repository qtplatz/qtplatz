// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
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

#include "time_data_reader.hpp"
#include <adfs/sqlite.hpp>
#include <adcontrols/countingdata.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <algorithm>
#include <fstream>
#include <iostream>

using namespace adtextfile;

time_data_reader::time_data_reader()
{
}

bool
time_data_reader::load( const std::string& name )
{
	boost::filesystem::path path( name );

	boost::filesystem::ifstream in( path );
    if ( in.fail() ) 
        return false;

    typedef boost::char_separator<char> separator;
    typedef boost::tokenizer< separator > tokenizer;

    separator sep( ", \t", "", boost::drop_empty_tokens );
    do {
        std::string line;
        if ( std::getline( in, line ) ) {
            if ( line.at( 0 ) == '#' )
                continue;

            tokenizer tokens( line, sep );
            adcontrols::CountingData data;
            adcontrols::CountingPeak peak;
            
            char * end;

            int col = 0;
            for ( auto it = tokens.begin(); it != tokens.end(); ++it, ++col ) {
                if ( col < 7 ) {
                    switch( col ) {
                    case 0:
                        data = adcontrols::CountingData();
                        peak = adcontrols::CountingPeak();
                        data.setTriggerNumber( std::strtol( it->c_str(), &end, 10 ) );   // trig#
                        break;
                    case 1:
                        data.setProtocolIndex( std::strtol( it->c_str(), &end, 10 ) );   // prot#
                        break;
                    case 2:
                        data.setElapsedTime( std::strtod( it->c_str(), &end ) );         // timestamp
                        break;
                    case 3:
                        data.setTimeSinceEpoch( std::strtoll( it->c_str(), &end, 10 ) ); // epock time
                        break;
                    case 4:
                        data.setEvents( std::strtol( it->c_str(), &end, 16 ) );          // events
                        break;
                    case 5:
                        data.setThreshold( std::strtod( it->c_str(), &end ) / 1000 );    // threshold (mV)->(V)
                        break;
                    case 6:
                        data.setAlgo( std::strtol( it->c_str(), &end, 10 ) ); // algo
                        break;
                    default:
                        break;
                    }

                } else {
                    int pcol = ( col - 7 ) % 6;
                    switch( pcol ) {
                    case 0:
                        peak = adcontrols::CountingPeak();
                        peak.apex.first = std::strtod( it->c_str(), &end );
                        break;
                    case 1:
                        peak.apex.second = std::strtod( it->c_str(), &end );
                        break;
                    case 2:
                        peak.front.first = std::strtod( it->c_str(), &end );
                        break;
                    case 3:
                        peak.front.second = std::strtod( it->c_str(), &end );
                        break;
                    case 4:
                        peak.back.first = std::strtod( it->c_str(), &end );
                        break;
                    case 5:
                        peak.back.second = std::strtod( it->c_str(), &end );
                        break;                                                                        
                    }
                    if ( pcol == 5 )
                        data.peaks().emplace_back( peak );
                }
            }
            data_.emplace_back( data );
        }

    } while( ! in.eof() );

    return true;
}

const std::vector< adcontrols::CountingData >&
time_data_reader::data() const
{
    return data_;
}

bool
time_data_reader::is_time_data( const std::string& path, std::string& adfsname )
{
    std::string::size_type pos;
    if ( ( pos = path.find( "_time_data.txt" ) ) != std::string::npos ) {
        boost::filesystem::path adfile( path.substr( 0, pos ) + ".adfs" );
        if ( boost::filesystem::exists( adfile ) )
            adfsname = adfile.string();
        return true;
    }
    return false;
}

//static
bool
time_data_reader::readScanLaw( const std::string& adfsname
                               , double& accelVoltage, double& tDelay, double& fLength
                               , std::string& spectrometer )
{
    adfs::sqlite db;

    if ( db.open( adfsname.c_str(), adfs::readonly ) ) {
        adfs::stmt sql( db );
        
        sql.prepare( "SELECT objtext,acclVoltage,tDelay,fLength,spectrometer FROM ScanLaw,Spectrometer WHERE clsidSpectrometer = id" );
        int rows( 0 );
        while( sql.step() == adfs::sqlite_row ) {
            auto objtext = sql.get_column_value< std::string >( 0 );
            accelVoltage = sql.get_column_value< double >( 1 );
            tDelay       = sql.get_column_value< double >( 2 );
            fLength      = sql.get_column_value< double >( 3 );
            spectrometer = sql.get_column_value< std::string >( 4 );
            if ( objtext.find( "timecount" ) != std::string::npos )
                return true;
            ++rows;
        }
        return rows;
    }
    return false;
}
