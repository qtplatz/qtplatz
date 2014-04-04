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
#include <boost/filesystem/fstream.hpp>
#include <vector>

using namespace galactic;

datafile::datafile()
{
	//memset(&acqu_, 0, sizeof( acqu_ ) );
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
		//double fraction = acqu_.fMax - acqu_.fMax * idx / n + acqu_.ml2;
		//double mz = acqu_.ml1 / fraction;
		double mz = 0;
		pMS->setMass( idx, mz );
	}
	pMS->setAcquisitionMassRange( 0, 0 ); //acqu_.mlow, acqu_.mhigh );
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
datafile::getSpectrum( int /* fcn*/, int /*idx*/, adcontrols::MassSpectrum&, uint32_t ) const
{
	return false;
}

/////////////////////////

bool
datafile::_open( const std::wstring& filename, bool )
{
	boost::filesystem::path path( filename );
	boost::filesystem::ifstream in( path, std::ios_base::binary );

	if ( in.fail() )
		return false;

    portfolio::Portfolio portfolio;
	portfolio.create_with_fullpath( filename );
    bool res = false;
#if 0    
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
#endif
    return res;
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
	boost::filesystem::path path( filename );
	if ( path.extension() == L".spc" || path.extension() == L".SPC" )
		return true;
	return false;
}

