/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
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

#include "fsio.hpp"
#include "adfsio.hpp"
#include "cpio.hpp"
#include <adcontrols/datafile.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/msassignedmass.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adinterface/method.hpp>
#include <adfs/cpio.hpp>
#include <adportable/debug.hpp>

using namespace adutils;

fsio::fsio()
{
}

bool
fsio::mount( adfs::filesystem& fs, const std::wstring& os_filename )
{
    try {
        return fs.mount( os_filename.c_str() );
    } catch ( adfs::exception& ) {
    }
    return false;
}

bool
fsio::create( adfs::filesystem& fs, const std::wstring& os_filename )
{
    try {
        return fs.create( os_filename.c_str() );
    } catch ( adfs::exception& ) {
    }
    return false;
}

bool
fsio::save(  adfs::filesystem& fs, const adcontrols::MassSpectrum& t, const std::wstring& id, const std::wstring& folder_name )
{
    adfs::folder folder = fs.addFolder( folder_name );
    adfs::file file = folder.addFile( id );
    return file.save( t ); // adfs::cpio< adcontrols::MassSpectrum >::save( t, file );
}

bool
fsio::save( adfs::filesystem& fs, const adcontrols::MSCalibrateResult& t, const std::wstring& id, const std::wstring& folder_name )
{
    adfs::folder folder = fs.addFolder( folder_name );
    return adfsio< adcontrols::MSCalibrateResult >::write( folder, t, id );
}

bool
fsio::load( adfs::filesystem& fs, adcontrols::MassSpectrum& t, const std::wstring& id, const std::wstring& folder_name )
{
    adfs::folder folder = fs.findFolder( folder_name );
    if ( folder.files().empty() )
        return false;

    std::vector< adfs::file > files = folder.files();
    auto it = std::find_if( files.begin(), files.end(), [=]( const adfs::file& f ){ return f.name() == id; });
    if ( it != files.end() )
        return it->fetch( t ); // adfs::cpio< adcontrols::MassSpectrum >::load( t, *it );
    return false;
}

bool
fsio::load( adfs::filesystem& fs, adcontrols::MSCalibrateResult& t, const std::wstring& id, const std::wstring& folder_name )
{
    adfs::folder folder = fs.findFolder( folder_name );
    if ( folder.files().empty() )
        return false;
    return adfsio< adcontrols::MSCalibrateResult >::read( folder, t, id );
}

bool
fsio::save_mscalibfile( adfs::filesystem& fs, const adcontrols::MSCalibrateResult& d )
{
    return save( fs, d, d.dataClass(), L"/MSCalibration" );
}

bool
fsio::save_mscalibfile( adfs::filesystem& fs, const adcontrols::MassSpectrum& ms )
{
    return save( fs, ms, ms.dataClass(), L"/MSCalibration" );
}

bool
fsio::load_mscalibfile( adfs::filesystem& fs, adcontrols::MSCalibrateResult& d )
{
    return load( fs, d, d.dataClass(), L"/MSCalibration" );
}

bool
fsio::load_mscalibfile( adfs::filesystem& fs, adcontrols::MassSpectrum& ms )
{
    return load( fs, ms, ms.dataClass(), L"/MSCalibration" );
}

bool
fsio::save( adfs::filesystem& fs, const adinterface::Method& t, const std::wstring& id, const std::wstring& folder_name)
{
    adfs::folder folder = fs.addFolder( folder_name );
    return adfsio< adinterface::Method >::write( folder, t, id );
}

bool
fsio::load( adfs::filesystem& fs, adinterface::Method& t, const std::wstring& id, const std::wstring& folder_name )
{
    adfs::folder folder = fs.findFolder( folder_name );
    if ( folder.files().empty() )
        return false;
    return adfsio< adinterface::Method >::read( folder, t, id );
}

