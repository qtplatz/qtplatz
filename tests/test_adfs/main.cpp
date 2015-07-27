// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
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

#include <adfs/adfs.hpp>
#include <adfs/cpio.hpp>
#include <adfs/sqlite.hpp>
#include <adcontrols/massspectrum.hpp>
#include <portfolio/portfolio.hpp>
#include <boost/filesystem.hpp>

std::wstring path( L"test.adfs" );

bool
test_create()
{
    adfs::filesystem fs;
    adcontrols::MassSpectrum ms;
    ms.resize( 1000 );

	if ( ! boost::filesystem::exists( path ) ) {
		if ( ! fs.create( path.c_str() ) )
			return false;
	} else {
		if ( ! fs.mount( path.c_str() ) )
			return false;
	}
	adfs::folder folder = fs.addFolder( L"/Processed/Spectra" );

    if ( folder ) {
		adfs::file file = folder.addFile( adfs::create_uuid(), L"title" );
        if ( file ) {
            file.dataClass( ms.dataClass() );
            auto id = file.id();
            if ( file.save( ms ) ) //adfs::cpio< adcontrols::MassSpectrum >::save( ms, file ) )
				file.commit();
        }
	}
    return true;
}


int
main(int argc, char *argv[])
{
    (void)(argc);
    (void)(argv);
    std::cout << "sizeof( int ) = " << sizeof(int) << std::endl;    
    std::cout << "sizeof( long ) = " << sizeof(long) << std::endl;
    std::cout << "sizeof( long long ) = " << sizeof(long long) << std::endl;
    std::cout << "sizeof( uint64_t ) = " << sizeof(uint64_t) << std::endl;        

    test_create();
}
