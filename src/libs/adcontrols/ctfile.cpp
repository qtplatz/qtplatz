/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
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

#include "ctfile.hpp"
#include "ctable.hpp"
#include <boost/filesystem/fstream.hpp>
#include <boost/tokenizer.hpp>
#include <adportable/string.hpp>

using namespace adcontrols;

CTFile::~CTFile()
{
}

CTFile::CTFile()
{
}

bool
CTFile::load_molfile( const boost::filesystem::path& path, CTable& ctable )
{
	ctable.clear();
	boost::filesystem::ifstream inf( path );

	std::vector< std::string > header;
	std::string line;
	for ( int i = 0; i < 3; ++i ) {
		std::getline( inf, line );
		header.push_back( line );
	}

	typedef boost::char_separator< char > separator;
	typedef boost::tokenizer< separator > tokenizer;
	separator sep( ", \t", "", boost::drop_empty_tokens );
 
	// count block
	int natoms = 0, nbounds = 0;
    int chiral = 0;
    int nprops = 0;
	std::string version;

	if ( std::getline( inf, line ) ) {
		tokenizer tokens( line, sep );
		tokenizer::iterator it = tokens.begin();
		if ( it != tokens.end() )
			natoms = atoi( it->c_str() );
        if ( ++it != tokens.end() )
			nbounds = atoi( it->c_str() );
		++it; // number of atom lists
		++it; // fff obsolete
		if ( ++it != tokens.end() ) // ccc chiral flag
			chiral = atoi( it->c_str() );
		++it; // obsolete
		++it; // obsolete
		++it; // obsolete
		++it; // obsolete
		if ( ++it != tokens.end() ) // mmm number of lines of additional properties
			nprops = atoi( it->c_str() );
        if ( ++it != tokens.end() )
			version = *it;
	}

	// atoms block
	for ( int i = 0; i < natoms; ++i ) {
		CTable::Atom a;
		if ( std::getline( inf, line ) ) {
			tokenizer tokens( line, sep );
			tokenizer::iterator it = tokens.begin();
            if ( it != tokens.end() )
				a.x = atof( it->c_str() );
            if ( ++it != tokens.end() )
				a.y = atof( it->c_str() );
            if ( ++it != tokens.end() )
				a.z = atof( it->c_str() );
            if ( ++it != tokens.end() )
				a.symbol = adportable::string::convert( *it );
			if ( ++it != tokens.end() )
				a.mass_difference = atoi( it->c_str() );
			if ( ++it != tokens.end() )
				a.charge = atoi( it->c_str() );
			if ( ++it != tokens.end() )
				a.atom_stereo_parity = atoi( it->c_str() );
			if ( ++it != tokens.end() )
				a.hydrogen_count = atoi( it->c_str() );
			if ( ++it != tokens.end() )
				a.stereo_care_box = atoi( it->c_str() );
			if ( ++it != tokens.end() )
				a.valence = atoi( it->c_str() );
			if ( ++it != tokens.end() )
				a.H0 = atoi( it->c_str() );
			++it;
			++it;
			if ( ++it != tokens.end() )
				a.atom_atom_mapping_number = atoi( it->c_str() );
			if ( ++it != tokens.end() )
				a.inversion_retention_flag = atoi( it->c_str() );
			if ( ++it != tokens.end() )
				a.exact_change_flag = atoi( it->c_str() );
		}
		ctable << a;
	}

	for ( int i = 0; i < nbounds; ++i ) {
		CTable::Bond b;
		if ( std::getline( inf, line ) ) {
			tokenizer tokens( line, sep );
			tokenizer::iterator it = tokens.begin();
            if ( it != tokens.end() )
				b.first_atom_number = atoi( it->c_str() );
            if ( ++it != tokens.end() )
				b.second_atom_number = atoi( it->c_str() );
            if ( ++it != tokens.end() )
				b.bond_type = atoi( it->c_str() );
            if ( ++it != tokens.end() )
				b.bond_stereo = atoi( it->c_str() );
			++it;
			if ( ++it != tokens.end() )
				b.bond_topology = atoi( it->c_str() );
			if ( ++it != tokens.end() )
				b.reacting_center_status = atoi( it->c_str() );
		}
		ctable << b;
	}
	return true;
}