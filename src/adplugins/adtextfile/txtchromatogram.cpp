/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include "txtchromatogram.hpp"
#include <adportable/debug.hpp>
#include <adportable/textfile.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/numeric/interval.hpp>
#include <boost/tokenizer.hpp>
#include <algorithm>
#include <fstream>

using namespace adtextfile;

TXTChromatogram::TXTChromatogram()
{
}

bool
TXTChromatogram::load( const std::wstring& name )
{
	boost::filesystem::path path( name );

	boost::filesystem::ifstream in( path );
    if ( in.fail() ) 
        return false;

    typedef boost::char_separator<char> separator;
    typedef boost::tokenizer< separator > tokenizer;

    separator sep( ", \t", "", boost::drop_empty_tokens );

    std::vector<double> timeArray, intensArray;

	if ( path.extension() == ".csv" ) {
        
        auto chr = std::make_shared< adcontrols::Chromatogram >();
        if ( compile_header( in ) ) {

            do {
                double values[2];
                std::string line;
                if ( adportable::textfile::getline( in, line ) ) {
                    tokenizer tokens( line, sep );
                    
                    int i = 0;
                    for ( tokenizer::iterator it = tokens.begin(); it != tokens.end() && i < 3; ++it, ++i ) {
                        const std::string& s = *it;
                        values[i] = atof( s.c_str() );
                    }
                    if ( i == 2 ) {
                        timeArray.push_back( values[ 0 ] );
                        intensArray.push_back( values[ 1 ] );
                    }
                }
            } while( ! in.eof() );
            chr->resize( intensArray.size() );
            chr->setIntensityArray( intensArray.data() );
            chr->setTimeArray( timeArray.data() );

            chromatograms_.push_back( chr );

            return true;
        }
    }

    return false;
}

bool
TXTChromatogram::compile_header( std::ifstream& in )
{
    std::streampos pos = in.tellg();

    // std::string source_path;
    // double lower_mass(0), upper_mass(0);
    // double elapsed_time(0);

    std::string line;

    if ( adportable::textfile::getline( in, line ) ) {

		if ( line.find( "mzlist path" ) == line.npos )
            return false;
    }

    while ( adportable::textfile::getline( in, line ) ) {
        if ( line.find( "Elapsed time" ) != line.npos )
            return true;
    }

    in.seekg( pos );
    return false;
}

