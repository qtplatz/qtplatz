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

#include "datafile_factory.hpp"
#include "datafile.hpp"
#include <adcontrols/processeddataset.hpp> // for delition of scoped_ptr<ProcessedDataset>
#include <adplugin/visitor.hpp>

using namespace spcfile;

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

const char *
datafile_factory::mimeTypes() const
{
	return 
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\
<mime-info xmlns='http://www.freedesktop.org/standards/shared-mime-info'>\n\
  <mime-type type=\"application/d\">\n\
    <sub-class-of type=\"application/octet-stream\"/>\n\
	<comment>Grams SPC file</comment>\n\
    <glob pattern=\"*.spc\"/>\n\
  </mime-type>\n\
  </mime-info>\n";
}

const wchar_t *
datafile_factory::name() const
{
    return L"Galactic SPC File";
}

bool
datafile_factory::access( const wchar_t * filename, adcontrols::access_mode ) const
{
	return datafile::is_valid_datafile( filename );
}

adcontrols::datafile *
datafile_factory::open( const wchar_t * filename, bool readonly ) const
{
	spcfile::datafile * p = new spcfile::datafile();

	if ( p->_open( filename, readonly ) )
		return p;
	delete p;

    return 0;
}

////////////////////////////////////////////
// adplugin::plugin implementation

const char *
datafile_factory::iid() const 
{
    return "com.ms-cheminfo.qtplatz.adplugins.datafile_factory.spcfile";
}

void
datafile_factory::accept( adplugin::visitor& v, const char * adplugin )
{
	v.visit( this, adplugin );
}

void *
datafile_factory::query_interface_workaround( const char * typenam )
{
    if ( std::string( typenam ) == typeid( adcontrols::datafile_factory ).name() )
        return static_cast< adcontrols::datafile_factory *>(this);
    return 0;
}
