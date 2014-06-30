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

#include "adfile.hpp"
#include "fsio2.hpp"
#include <adfs/filesystem.hpp>
#include <boost/filesystem.hpp>

using namespace adutils;

adfile::~adfile()
{
}

adfile::adfile()
{
}

adfile::adfile( const boost::filesystem::path& filename )
{
    open( filename );
}

bool
adfile::open( const boost::filesystem::path& filename )
{
    if ( std::shared_ptr< adfs::filesystem > fs = std::make_shared< adfs::filesystem >() ) {
        if ( fsio2::open( *fs, filename.wstring() ) ) {
            fs_ = fs;
            return true;
        }
    }
    return false;
}

adfile::operator bool () const
{
    return fs_ != 0;
}

bool
adfile::append( const portfolio::Folium& folium, const adcontrols::datafile& datasource )
{
    if ( fs_ ) 
        return fsio2::append( *fs_, folium, datasource );
    return false;
}
