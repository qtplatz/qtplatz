/**************************************************************************
** Copyright (C) 2010-2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2019 MS-Cheminformatics LLC
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

#include "document.hpp"
#include <adcontrols/massspectrum.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/folder.hpp>
#include <adfs/file.hpp>
#include <adfs/adfs.hpp>
#include <adfs/cpio.hpp>
#include <adportable/debug.hpp>
#include <boost/filesystem.hpp>

using namespace tools;

bool
document::appendOnFile( const boost::filesystem::path& path
                        , const QString& title
                        , const adcontrols::MassSpectrum& ms
                        , QString& id )
{
    adfs::filesystem fs;
    
	if ( ! boost::filesystem::exists( path ) ) {
		if ( ! fs.create( path.c_str() ) )
			return false;
	} else {
		if ( ! fs.mount( path.c_str() ) )
			return false;
	}
	adfs::folder folder = fs.addFolder( L"/Processed/Spectra" );
    
    if ( folder ) {
		adfs::file file = folder.addFile( adfs::create_uuid(), title.toStdWString() );
        if ( file ) {
            file.dataClass( ms.dataClass() );
            id = QString::fromStdWString( file.id() );
            if ( file.save( ms ) )
				file.commit();
        }
	}
    return true;
}

std::shared_ptr< adcontrols::MassSpectrum >
document::histogram( std::vector< size_t >& hist, const adcontrols::MassSpectrum& ms, double v_th )
{
    if ( hist.empty() ) {
        hist.resize( ms.size() );
        std::fill( hist.begin(), hist.end(), 0 );
    }

    auto prev = ms.getIntensity( 0 );
    for ( size_t i = 1; i < ms.size(); ++i ) {
        auto d = ms.getIntensity( i );
        if ( prev < v_th && v_th < d )
            hist[i]++;
        prev = d;
    }
    
    std::vector< double > times, masses, intens;
    for ( size_t i = 0; i < hist.size(); ++i ) {
        if ( hist[ i ] ) {
            times.emplace_back( ms.getTime( i ) );
            masses.emplace_back( ms.getMass( i ) );
            intens.emplace_back( hist[ i ] );
            //ADDEBUG() << "[" << i << "]\t" << ms.getTime( i ) << ", " << ms.getMass( i ) << ", " << hist[i];
        }
    }

    auto pkd = std::make_shared< adcontrols::MassSpectrum >();
    pkd->clone( ms );
    pkd->setCentroid( adcontrols::CentroidHistogram );
    pkd->resize( times.size() );

    pkd->setTimeArray( std::move( times ) );
    pkd->setMassArray( std::move( masses ) );
    pkd->setIntensityArray( std::move( intens ) );

    return pkd;
}
