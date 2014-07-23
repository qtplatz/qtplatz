/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include "quandatawriter.hpp"
#include <adcontrols/datafile.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/datasubscriber.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/processeddataset.hpp>
#include <adcontrols/quanmethod.hpp>
#include <adcontrols/quancompounds.hpp>
#include <adcontrols/quansequence.hpp>
#include <portfolio/portfolio.hpp>
#include <portfolio/folder.hpp>
#include <portfolio/folium.hpp>
#include <adfs/adfs.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/folder.hpp>
#include <adfs/file.hpp>
#include <adfs/cpio.hpp>

#include <adportable/profile.hpp>
#include <boost/filesystem.hpp>

using namespace quan;

QuanDataWriter::~QuanDataWriter()
{
}

QuanDataWriter::QuanDataWriter( const std::wstring& path ) : path_( path ) 
{
}

bool
QuanDataWriter::open()
{
    if ( !boost::filesystem::exists( path_ ) ) {
        if ( !fs_.create( path_.c_str() ) )
            return false;
    } else {
        if ( !fs_.mount( path_.c_str() ) )
            return false;
    }
    return true;
}

adfs::file
QuanDataWriter::write( const adcontrols::MassSpectrum& ms, const std::wstring& tittle )
{
    if ( adfs::folder folder = fs_.addFolder( L"/Processed/Spectra" ) ) {
        if ( adfs::file file = folder.addFile( adfs::create_uuid(), tittle ) ) {
            file.dataClass( ms.dataClass() );
            if ( adfs::cpio< adcontrols::MassSpectrum >::save( ms, file ) )
                file.commit();
            return file;
        }
    }
    return adfs::file();
}
