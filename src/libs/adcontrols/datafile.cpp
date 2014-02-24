/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
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
#include "datafilebroker.hpp"

using namespace adcontrols;

// static
bool
datafile::access( const std::wstring& filename )
{
    return datafileBroker::access( filename );
}

datafile*
datafile::create( const std::wstring& filename )
{
    datafile * file = datafileBroker::create( filename );
    if ( file ) {
        file->filename_ = filename;
        file->readonly_ = false;
    }
    return file;
}

datafile*
datafile::open( const std::wstring& filename, bool readonly )
{
    datafile * file = datafileBroker::open( filename, readonly );
    if ( file ) {
        file->filename_ = filename;
        file->readonly_ = readonly;
    }
    return file;
}

void
datafile::close( datafile *& file )
{
    delete file;
    file = 0;
}

const std::wstring&
datafile::filename() const
{
    return filename_;
}

bool
datafile::readonly() const
{
    return readonly_;
}

//---------------
bool
datafile::saveContents( const std::wstring&, const portfolio::Portfolio&, const datafile& )
{
	return false;
}

bool
datafile::saveContents( const std::wstring&, const portfolio::Portfolio& )
{
	return false;
}
