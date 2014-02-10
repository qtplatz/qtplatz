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

#include "protease.hpp"
#include <algorithm>

using namespace adprot;

protease::protease( const std::string& name ) : name_( name )
{
    if ( name_ == "trypsin" )
        cleave_points( "KA" ); // Carboxyl side of Lys, Arg
}

const std::string&
protease::name() const
{
    return name_;
}

const std::vector< std::string >&
protease::aliases() const
{
    return aliases_;
}

std::vector< std::string >&
protease::aliases()
{
    return aliases_;
}

void 
protease::cleave_points( const char * aa )
{
    if ( aa ) {
        while ( *aa )
            cleave_points_.push_back( *aa++ );

        std::sort( cleave_points_.begin(), cleave_points_.end() );
        auto it = std::unique( cleave_points_.begin(), cleave_points_.end() );

        if ( it != cleave_points_.end() )
            cleave_points_.erase( it );
    }
}

const std::string&
protease::cleave_points() const
{
    return cleave_points_;
}

//static
bool
protease::digest( const protease& enzyme, const std::string& sequence, std::vector< std::string >& output )
{
    output.clear();

    std::string::size_type bpos = 0;
    std::string::size_type pos = 0;
    while ( ( pos = sequence.find_first_of( enzyme.cleave_points(), bpos ) ) != std::string::npos ) {
        std::string digested = sequence.substr( bpos, pos - bpos + 1);
        output.push_back( digested );
        bpos = pos + 1;
    }
    output.push_back( sequence.substr( bpos ) );

    return !output.empty();
}

//static
bool
protease::digest( const protease& enzyme, const std::string& sequence, std::vector< size_t >& output )
{
    output.clear();

    std::string::size_type bpos = 0;
    std::string::size_type pos = 0;
    while ( ( pos = sequence.find_first_of( enzyme.cleave_points(), bpos ) ) != std::string::npos ) {
        output.push_back( pos );
        bpos = pos + 1;
    }

    return !output.empty();
}

//static
bool
protease::digest( const protease& enzyme, const std::string& sequence, std::string& richText )
{
    struct tagger {
        bool tag_;

        tagger() : tag_( true ) {}
        void operator()( std::string& text, const std::string& rhs ) {
			const char * btag = "<span style='background-color: lightcyan'>";
			const char * etag = "</span>";
            if ( tag_ )
                text += btag;
            text += rhs;
            if ( tag_ )
                text += etag;
            tag_ = !tag_;
        }
    };

    richText.clear();
    tagger tag;

    std::string::size_type bpos = 0;
    std::string::size_type pos = 0;
    while ( ( pos = sequence.find_first_of( enzyme.cleave_points(), bpos ) ) != std::string::npos ) {
        tag( richText, sequence.substr( bpos, pos - bpos + 1 ) );
		bpos = pos + 1;
	}

    tag( richText, sequence.substr( bpos ) );

    return true;
}

