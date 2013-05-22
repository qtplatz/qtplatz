/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Copyright (C) MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
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

#include "datafile_factory.hpp"
#include "datafile.hpp"
#include <adcontrols/datafile.hpp>
#include <adcontrols/processeddataset.hpp> // for delition of scoped_ptr<ProcessedDataset>

using namespace mzxml;

datafile_factory::datafile_factory()
{
	// std::map< std::string, std::string > map;
	// jcampdxparser::parse_file( map, L"C:/Users/thondo/Documents/Osaka-U/FTMS-DATA/100-TNT+RDX-AfterCalib/1/acqu" );
}

datafile_factory::~datafile_factory(void)
{
}

void
datafile_factory::close( adcontrols::datafile * p )
{
   delete p;
}

const std::wstring&
datafile_factory::name() const
{
    static std::wstring name( L"mzXML" );
    return name;
}

bool
datafile_factory::access( const std::wstring& filename, adcontrols::access_mode ) const
{
	return datafile::is_valid_datafile( filename );
}

adcontrols::datafile *
datafile_factory::open( const std::wstring& filename, bool readonly ) const
{
/*
	fticr::datafile * p = new fticr::datafile();

	if ( p->_open( filename, readonly ) )
		return p;
	delete p;
*/
    return 0;
}
