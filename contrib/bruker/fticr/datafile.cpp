/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Copyright (C) MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: toshi.hondo@MS-Cheminformatics.com
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
#include <adportable/float.hpp>
#include <adportfolio/portfolio.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
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

static double
tof2mass( double tof, double c1, double c2, double c3 )
{
    double A = c3;
    double B = std::sqrt( 1e12 / c1 );
    double C = c2 - tof;
    if ( adportable::compare<double>::essentiallyEqual( A, 0 ) )
        return ( C * C ) / ( B *  B );
    else {
        double m2 = ( ( -B + sqrt( ( B * B ) - ( 4 * A * C ) ) ) / ( 2 * A ) );
        return m2 * m2;
    }

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
#if 0 // original code
		double fraction = acqu_.fMax - acqu_.fMax * idx / n + acqu_.ml2;
		double mz = acqu_.ml1 / fraction;
#endif
        //double tof = (acqu_.delay * 1.0e-9) + idx * (acqu_.dw * 1.0e-9);
        double tof = (acqu_.delay) + idx * (acqu_.dw);
        double mz = tof2mass( tof, acqu_.ml1, acqu_.ml2, acqu_.ml3 );
		pMS->setMass( idx, mz );
	}
    pMS->setAcquisitionMassRange( pMS->getMass( 0 ), pMS->getMass( pMS->size() - 1 ) );
	//pMS->setAcquisitionMassRange( acqu_.mlow, acqu_.mhigh );
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
datafile::getSpectrum( int /* fcn*/, size_t /*idx*/, adcontrols::MassSpectrum&, uint32_t ) const
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

    portfolio::Portfolio portfolio;
	portfolio.create_with_fullpath( filename_ );
    
    bool res = false;
    if ( boost::filesystem::is_regular_file( dw.root_dir / L"acqu" ) ) {
        res = _1open( filename_, portfolio );
    } else {
        boost::filesystem::directory_iterator end;
        for ( boost::filesystem::directory_iterator it( filename_ ); it != end; ++it ) {
            if ( boost::filesystem::is_directory( *it / L"pdata" ) )
                res = _1open( it->path().wstring(), portfolio );
        }
    }
    if ( res ) {
        processedDataset_.reset( new adcontrols::ProcessedDataset );
        processedDataset_->xml( portfolio.xml() );
    }
    return res;
}

bool
datafile::_1open( const std::wstring& filename, portfolio::Portfolio& portfolio )
{
    boost::filesystem::path _1( filename );  // "a_file/1/acqu"

    boost::filesystem::path acqu( _1 / L"acqu" );

	if ( boost::filesystem::is_regular_file( acqu ) ) {
		jcampdxparser::vector_type map;
		if ( jcampdxparser::parse_file( map, acqu.wstring() ) ) {
            if ( map.find( "$ML1" ) != map.end() )
                try { acqu_.ml1 = boost::lexical_cast<double>( map [ "$ML1" ] ); } catch ( boost::bad_lexical_cast& ) { return false; }
            if ( map.find( "$ML2" ) != map.end() )
                try { acqu_.ml2 = boost::lexical_cast<double>( map [ "$ML2" ] ); } catch ( boost::bad_lexical_cast& ) { return false; }
            if ( map.find( "$ML3" ) != map.end() )
                try { acqu_.ml3 = boost::lexical_cast<double>( map [ "$ML3" ] ); } catch ( boost::bad_lexical_cast& ) { return false; }
            if ( map.find( "$SW_h" ) != map.end() )
                try { acqu_.fMax = boost::lexical_cast<double>( map [ "$SW_h" ] ); } catch ( boost::bad_lexical_cast& ) { return false; }
            if ( map.find( "$NS" ) != map.end() )
                try { acqu_.ns = boost::lexical_cast<int>( map [ "$NS" ] ); } catch ( boost::bad_lexical_cast& ) { return false; }
            if ( map.find( "$MW_high" ) != map.end() )
                try { acqu_.mhigh = boost::lexical_cast<double>( map [ "$MW_high" ] ); } catch ( boost::bad_lexical_cast& ) { return false; }
            if ( map.find( "$MW_low" ) != map.end() )
                try { acqu_.mlow = boost::lexical_cast<double>( map [ "$MW_low" ] ); } catch ( boost::bad_lexical_cast& ) { return false; }

            if ( map.find( "$TD" ) != map.end() )
                try { acqu_.td = boost::lexical_cast<double>( map [ "$TD" ] ); } catch ( boost::bad_lexical_cast& ) { return false; }
            if ( map.find( "$DELAY" ) != map.end() )
                try { acqu_.delay = boost::lexical_cast<double>( map [ "$DELAY" ] ); } catch ( boost::bad_lexical_cast& ) { return false; }
            if ( map.find( "$DW" ) != map.end() )
                try { acqu_.dw = boost::lexical_cast<double>( map [ "$DW" ] ); } catch ( boost::bad_lexical_cast& ) { return false; }
		}
	}

	portfolio::Folder spectra = portfolio.addFolder( L"Spectra" );

	//boost::filesystem::directory_iterator pos( dw.pdata() );
	boost::filesystem::directory_iterator last;

    for ( boost::filesystem::directory_iterator pos( _1 / L"pdata" ); pos != last; ++pos ) {
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
    return true;
}

size_t
datafile::posFromTime( double ) const
{
	return 0;
}

double
datafile::timeFromPos( size_t ) const
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
    // "file/1/pdata"
	boost::filesystem::path path( file );

    if ( boost::filesystem::is_directory( path ) ) {

        if ( boost::filesystem::is_directory( path / L"pdata" ) ) {
            // if "1" selected
            root_dir = path;
            valid = true;

        } else {
            // if "file" selected
            boost::filesystem::directory_iterator end;
            for ( boost::filesystem::directory_iterator it( path ); it != end; ++it ) {
                if ( boost::filesystem::is_directory( *it / L"pdata" ) ) {
                    root_dir = path;
                    valid = true;
                    break;
                }
            }
        }

    } else {
        // if "acqu" (any regular file under "1" directory has been selected
        boost::filesystem::path dir( path.branch_path() );
        
        if ( boost::filesystem::is_directory( dir ) ) {
            if ( boost::filesystem::is_directory( dir / L"pdata" ) ) {
                root_dir = dir;
                valid = true;
            }
        }
    }
}
