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
#include "../spcfile/spcfile.hpp"
#include "../spcfile/spchdr.hpp"
#include "../spcfile/subhdr.hpp"
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
#include <boost/format.hpp>
#include <vector>

using namespace galactic;

datafile::datafile()
{
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
datafile::fetch( const std::wstring& foliumGuid, const std::wstring& dataType ) const
{
	(void)dataType;
	boost::any any;
    
    auto it = dataIds_.find( foliumGuid ); 

    if ( it != dataIds_.end() ) {

        auto ms = std::make_shared< adcontrols::MassSpectrum >();

        if ( getSpectrum( 0, int(it->second), *ms, 0 ) )
            any = ms;

    }
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
    return spcfile_->spchdr()->number_of_subfiles();
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
datafile::getSpectrum( int /* fcn*/, int idx, adcontrols::MassSpectrum& ms, uint32_t /* objid */) const
{
    if ( idx < spcfile_->number_of_subfiles() ) {

        const galactic::spchdr& hdr = *spcfile_->spchdr();
        const galactic::subhdr& sub = *spcfile_->subhdr( idx );

        std::pair< double, double > range = std::make_pair( hdr.ffirst(), hdr.flast() );
        const size_t npts = hdr.fnpts();
        ms.resize( npts );
        ms.setAcquisitionMassRange( range.first, range.second );
        
        for ( int i = 0; i < npts; ++i ) {
            ms.setMass( i, i * double(( range.second - range.first )) / ( npts - 1 ) + range.first );
            ms.setIntensity( i, sub[i] );
        }
        return true;
    }
	return false;
}

/////////////////////////

bool
datafile::_open( const std::wstring& filename, bool )
{
    boost::filesystem::path path( filename );
    
    if ( boost::filesystem::exists( path ) ) {

        size_t fsize = boost::filesystem::file_size( path );
        boost::filesystem::ifstream in( path, std::ios_base::binary );
        
        if ( spcfile_ = std::make_shared< galactic::spcfile >( in, fsize ) ) {

            portfolio::Portfolio portfolio;
            portfolio.create_with_fullpath( filename );

            auto folder = portfolio.addFolder( L"Spectra" );

            for ( size_t i = 0; i < spcfile_->number_of_subfiles(); ++i ) {
                // auto ms = std::make_shared< adcontrols::MassSpectrum >();
                auto folium = folder.addFolium( ( boost::wformat( L"Spectrum(%1%)" ) % ( i + 1 ) ).str() );
                folium.assign( boost::any(), adcontrols::MassSpectrum::dataClass() ); // set empty data, it will be fixed by 'fetch'
                dataIds_[ folium.id() ] = i;
            }

            processedDataset_.reset( new adcontrols::ProcessedDataset );
            processedDataset_->xml( portfolio.xml() );

            return true;

        }

    }
    return false;
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

