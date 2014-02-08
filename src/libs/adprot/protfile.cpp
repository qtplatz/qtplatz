/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "protfile.hpp"
#include <boost/filesystem.hpp>
#include <fstream>

namespace adprot {
    
    static bool
    getline( std::istream& in, std::string& line )
    {
        while( in.good() ) {
            int c = in.get();
            if ( c == '\r') {
                if ( in.get() != '\n' ) // whoops, its old mac file
					in.unget();
                return true;
            } else if ( c == '\n' ) {
                return true;
            }
            line.push_back( c );
        }
        return !line.empty();
    }

}

using namespace adprot;

protfile::protfile( const std::string& filename ) : filename_( filename )
{
	std::fstream fin( filename );

    if ( fin.is_open() ) {
        while ( fetch( fin ) )
            ;
    }
}

protfile::operator bool() const
{
    return !proteins_.empty();
}

size_t
protfile::size() const
{
    return proteins_.size();
}

std::vector< protein >::const_iterator
protfile::begin() const
{
    return proteins_.begin();
}

std::vector< protein >::iterator
protfile::begin()
{
    return proteins_.begin();
}

std::vector< protein >::const_iterator
protfile::end() const
{
    return proteins_.end();
}

std::vector< protein >::iterator
protfile::end()
{
    return proteins_.end();
}

bool
protfile::fetch( std::istream& inf )
{
    int c = inf.get();
    while ( c != '>' && inf.good() ) // find '>'
        c = inf.get();

    if ( c == '>' ) {
        std::string common_name;
        std::string sequence;
        if ( getline( inf, common_name ) ) {
            while ( inf.good() ) {
                c = inf.get();
                if ( c == '>' ) { // find next record
                    inf.unget();
                    break;
                } else if ( c == '\r' || c== '\n' ) {
                    continue;
                } else {
                    sequence.push_back( c );
                }
            }
            proteins_.push_back( protein( common_name, sequence ) );
            return true;
        }
    }
    return false;
}
