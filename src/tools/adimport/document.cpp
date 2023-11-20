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
#include <adplugins/adspectrometer/massspectrometer.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/folder.hpp>
#include <adfs/file.hpp>
#include <adfs/adfs.hpp>
#include <adfs/cpio.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/debug.hpp>
#include <compiler/boost/workaround.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <vector>

using namespace tools;

document *
document::instance()
{
    static document __instance;
    return &__instance;
}

bool
document::appendOnFile( const std::filesystem::path& path
                        , const QString& title
                        , const adcontrols::MassSpectrum& ms
                        , QString& id )
{
    adfs::filesystem fs;

	if ( ! std::filesystem::exists( path ) ) {
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

    auto prev = ms.intensity( 0 );
    for ( size_t i = 1; i < ms.size(); ++i ) {
        auto d = ms.intensity( i );
        if ( prev < v_th && v_th < d )
            hist[i]++;
        prev = d;
    }

    std::vector< double > times, masses, intens;
    for ( size_t i = 0; i < hist.size(); ++i ) {
        if ( hist[ i ] ) {
            times.emplace_back( ms.time( i ) );
            masses.emplace_back( ms.mass( i ) );
            intens.emplace_back( hist[ i ] );
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

bool
document::initStorage( const boost::uuids::uuid& uuid, adfs::sqlite& db ) const
{
    return true;
}

bool
document::prepareStorage( adfs::filesystem& fs ) const
{
    adfs::stmt sql( fs.db() );

    sql.exec(
        "CREATE TABLE trigger ("
        " id INTEGER PRIMARY KEY"
        ", protocol INTEGER"
        ", timeSinceEpoch INTEGER"
        ", elapsedTime REAL"
        ", events INTEGER"
        ", threshold REAL"
        ", algo INTEGER"
        ", nbr_of_trigs INTEGER"
        ", raising_delta INTEGER"
        ", falling_delta INTEGER"
        ", front_end_range REAL"
        " )" );

    boost::uuids::uuid uuid = { 0 };

    if ( initStorage( uuid, fs.db() ) && uuid == boost::uuids::uuid{{ 0 }} ) {
        return true;
    }

    return false;
}

bool
document::closingStorage( const boost::uuids::uuid& uuid, adfs::filesystem& fs ) const
{
    if ( uuid == boost::uuids::uuid{{ 0 }} ) {
        // auto& fs = sp.filesystem();
    }
    return true;
}
