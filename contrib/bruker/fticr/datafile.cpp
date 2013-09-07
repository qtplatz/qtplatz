/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Copyright (C) MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
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

#include "datafile.hpp"
#include "jcampdxparser.hpp"
#include <adcontrols/datasubscriber.hpp>
#include <adcontrols/processeddataset.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <portfolio/portfolio.hpp>
#include <portfolio/folder.hpp>
#include <portfolio/folium.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/any.hpp>
#include <boost/lexical_cast.hpp>
#include <vector>

namespace fticr {
	struct dirwalk {
		boost::filesystem::path root_dir; // top directory name on windows
		bool valid;
		dirwalk( const std::wstring& file );
		inline operator bool () const { return valid; }
		inline boost::filesystem::path pdata() const { return root_dir / L"pdata"; }
	};
}

using namespace fticr;

datafile::datafile()
{
	memset(&acqu_, 0, sizeof( acqu_ ) );
}

//virtual
void
datafile::accept( adcontrols::dataSubscriber& sub )
{
    // AcquireDataset <LCMSDataset>
	sub.subscribe( *this );

    // subscribe processed dataset
	if ( processedDataset_ ) 
		sub.subscribe( *processedDataset_ );
}

// virtual
boost::any
datafile::fetch( const std::wstring& path, const std::wstring& dataType ) const
{
	boost::any any;
	boost::uint32_t x = 0;
	std::vector< double > intens;

    (void)dataType;

	boost::filesystem::path fpath( path );
	boost::uintmax_t n = boost::filesystem::file_size( path ) / 4;

	adcontrols::MassSpectrumPtr pMS( new adcontrols::MassSpectrum() );
    pMS->resize( static_cast< size_t >( n ) );

	boost::filesystem::ifstream rdfile( fpath, std::ios_base::binary );
	size_t idx;
	for ( idx = 0; idx < n && ! rdfile.eof(); ++idx ) {
		rdfile.read( reinterpret_cast<char *>(&x), sizeof(x) );
		pMS->setIntensity( idx, double( x ) );
		double fraction = acqu_.fMax - acqu_.fMax * idx / n + acqu_.ml2;
		double mz = acqu_.ml1 / fraction;
		pMS->setMass( idx, mz );
	}
	pMS->setAcquisitionMassRange( acqu_.mlow, acqu_.mhigh );
	any = pMS;     
	return any;
}

//virtual
adcontrols::datafile::factory_type
datafile::factory()
{ 
	return 0;
}

//virtual
size_t
datafile::getFunctionCount() const
{
	return 1;
}

//virtual
size_t
datafile::getSpectrumCount( int /* fcn */ ) const
{
	return 0;
}

//virtual
size_t
datafile::getChromatogramCount() const
{
	return 0;
}

//virtual
bool
datafile::getTIC( int /* fcn */, adcontrols::Chromatogram& ) const
{
	return false;
}

//virtual
bool
datafile::getSpectrum( int /* fcn*/, int /*idx*/, adcontrols::MassSpectrum& ) const
{
	return false;
}

/////////////////////////

bool
datafile::_open( const std::wstring& filename, bool )
{
    dirwalk dw( filename );
    if ( !dw )
		return false;
    filename_ = dw.root_dir.wstring();
	boost::filesystem::path acqu( dw.root_dir / L"acqu" );
	if ( boost::filesystem::is_regular_file( acqu ) ) {
		jcampdxparser::vector_type map;
		if ( jcampdxparser::parse_file( map, acqu.wstring() ) ) {
			try { acqu_.ml1   = boost::lexical_cast<double>( map[ "$ML1" ] );   } catch ( boost::bad_lexical_cast& ) { return false; }
			try { acqu_.ml2   = boost::lexical_cast<double>( map[ "$ML2" ] );   } catch ( boost::bad_lexical_cast& ) { return false; }
			try { acqu_.fMax  = boost::lexical_cast<double>( map[ "$SW_h" ] );  } catch ( boost::bad_lexical_cast& ) { return false; }
			try { acqu_.ns    = boost::lexical_cast<int>( map[ "$NS" ] );    } catch ( boost::bad_lexical_cast& ) { return false; }
			try { acqu_.mhigh = boost::lexical_cast<double>( map[ "$MW_high" ]);} catch ( boost::bad_lexical_cast& ) { return false; }
			try { acqu_.mlow  = boost::lexical_cast<double>( map[ "$MW_low" ] );} catch ( boost::bad_lexical_cast& ) { return false; }
		}
	}

	portfolio::Portfolio portfolio;
	portfolio.create_with_fullpath( filename_ );
	portfolio::Folder spectra = portfolio.addFolder( L"Spectra" );

	boost::filesystem::directory_iterator pos( dw.pdata() );
	boost::filesystem::directory_iterator last;

	for ( ; pos != last; ++pos ) {
		boost::filesystem::path p( *pos );
		if ( boost::filesystem::is_directory( p ) ) {
			boost::filesystem::path rdfile( p / L"1r" );
			if ( boost::filesystem::is_regular_file( rdfile ) ) {
				std::wstring title;
				if ( boost::filesystem::is_regular_file( p / L"title" ) ) {
					boost::filesystem::wifstream inf( p / L"title" );
                    inf >> title;
				}
				if ( title.empty() )
					title = L"Spectrum " + p.leaf().wstring();
				portfolio::Folium folium = spectra.addFolium( title );
				folium.setAttribute( L"dataType", L"MassSpectrum" );
				folium.id( rdfile.wstring() );
			}
		}
	}
	processedDataset_.reset( new adcontrols::ProcessedDataset );
	processedDataset_->xml( portfolio.xml() );
    return true;
}

size_t
datafile::posFromTime( double ) const
{
	return 0;
}

//static
bool
datafile::is_valid_datafile( const std::wstring& filename )
{
	dirwalk w( filename );
	return static_cast<bool>( w );
}


////////////////
dirwalk::dirwalk( const std::wstring& file ) : valid( false )
{
	boost::filesystem::path path( file );
	boost::filesystem::path dir( path.branch_path() );
    
	if ( boost::filesystem::is_directory( dir ) ) {
		if ( boost::filesystem::is_directory( dir / L"pdata" ) ) {
			root_dir = dir;
			valid = true;
		}
	}
}
