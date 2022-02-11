// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2022-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2022-2022 MS-Cheminformatics LLC
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

#include "txt_tokenizer.hpp"
#include <adportable/textfile.hpp>
#include <boost/tokenizer.hpp>
#include <string>

using namespace adtextfile;

txt_tokenizer::~txt_tokenizer()
{
}

txt_tokenizer::txt_tokenizer()
{
}

std::array< bool, 4 >
txt_tokenizer::load( std::ifstream& in
                     , data_type& data
                     , size_t skipLines
                     , std::vector< size_t >&& ignColumns
                     , bool hasTime
                     , bool hasMass
                     , bool isCentroid ) const
{
    typedef boost::char_separator<char> separator;
    typedef boost::tokenizer< separator > tokenizer;
    separator sep( ", \t", "", boost::drop_empty_tokens );

    size_t row(0);
    std::array< bool, 4 > flags = { hasTime, hasMass, true, isCentroid };
    do {
        std::string line;
        if ( adportable::textfile::getline( in, line ) ) {
            if ( skipLines ) {
                --skipLines;
                continue;
            }
            tokenizer tokens( line, sep );
            double values[4] = {0};
            int i(0);
            int col(0);
            for ( tokenizer::iterator it = tokens.begin(); it != tokens.end() && i < 4; ++it, ++col ) {
                const std::string& s = *it;
                if ( std::find( ignColumns.begin(), ignColumns.end(), col ) == ignColumns.end() )
                    values[i++] = atof( s.c_str() ); // boost::lexical_cast<double> in gcc throw bad_cast for "9999" format.
            }
            if ( i == 2 ) {
                if ( hasTime )
                    std::get< 0 >( data ).emplace_back( values[ 0 ] ); // time
                else
                    std::get< 1 >( data ).emplace_back( values[ 0 ] ); // mass
                std::get< 2 >( data ).emplace_back( values[ 1 ] * 1000 ); // intensity
                // cols[0].push_back( values[0] ); // (time|mass)
                // cols[1].push_back( values[1] * 1000 ); // intens
            } else if ( i >= 3 ) {
                std::get< 0 >( data ).emplace_back( values[ 0 ] );
                std::get< 1 >( data ).emplace_back( values[ 1 ] );
                std::get< 2 >( data ).emplace_back( values[ 2 ] );
                if ( flags[3] && i >= 4 ) {
                    std::get< 3 >( data ).emplace_back( int( values[ 3 ] + 0.5 ) ); // color (if centroid)
                }
                // cols[0].push_back( values[0] ); // time
                // cols[1].push_back( values[1] ); // mass
                // cols[2].push_back( values[2] ); // intens
            }
            ++row;
        }
    } while( ! in.eof() );
    return flags;
}
