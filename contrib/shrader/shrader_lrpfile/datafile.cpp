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
#include "../lrpfile/lrpfile.hpp"
#include "../lrpfile/lrptic.hpp"
#include "../lrpfile/msdata.hpp"
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/datasubscriber.hpp>
#include <adcontrols/processeddataset.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adportable/utf.hpp>
#include <adportfolio/portfolio.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/any.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/format.hpp>
#include <vector>

using namespace shrader;

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

boost::any
datafile::fetch( const std::string& foliumGuid, const std::string& dataType ) const
{
    return fetch( adportable::utf::to_wstring( foliumGuid ), adportable::utf::to_wstring( dataType ) );
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
    return lrpfile_->number_of_spectra();
}

//virtual
size_t
datafile::getChromatogramCount() const
{
	return 0;
}

//virtual
bool
datafile::getTIC( int /* fcn */, adcontrols::Chromatogram& c ) const
{
    if ( lrpfile_ ) {
        std::vector< double > time, intens;
        lrpfile_->getTIC( time, intens );
        c.resize( time.size() );
        c.setTimeArray( time.data() );
        c.setIntensityArray( intens.data() );
        return true;
    }
	return false;
}

//virtual
bool
datafile::getSpectrum( int /* fcn*/, size_t idx, adcontrols::MassSpectrum& ms, uint32_t /* objid */) const
{
    if ( lrpfile_ && unsigned( idx ) < lrpfile_->number_of_spectra() ) {

        if ( auto msdata = (*lrpfile_)[ idx ] ) {

            std::vector< double > time, intens;
            if ( lrpfile_->getMS( *msdata, time, intens ) ) {

                ms.resize( time.size() );
                ms.setMassArray( time.data() );
                ms.setIntensityArray( intens.data() );

                ms.setAcquisitionMassRange( time.front(), time.back() );

                return true;
            }
        }
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

        if ( (lrpfile_ = std::make_shared< shrader::lrpfile >( in, fsize )) ) {

            portfolio::Portfolio portfolio;
            portfolio.create_with_fullpath( filename );

            auto folder = portfolio.addFolder( L"Spectra" );
#if 0
            for ( size_t i = 0; i < spcfile_->number_of_subfiles(); ++i ) {
                // auto ms = std::make_shared< adcontrols::MassSpectrum >();
                auto folium = folder.addFolium( ( boost::wformat( L"Spectrum(%1%)" ) % ( i + 1 ) ).str() );
                folium.assign( boost::any(), adcontrols::MassSpectrum::dataClass() ); // set empty data, it will be fixed by 'fetch'
                dataIds_[ folium.id() ] = i;
            }
#endif
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
	if ( path.extension() == L".lrp" || path.extension() == L".LRP" )
		return true;
	return false;
}
