// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include "datafile_factory.hpp"
#include "datafile.hpp"
#include <adplugin/visitor.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

using namespace adtxtfactory;

datafile_factory::~datafile_factory(void)
{
}

datafile_factory::datafile_factory()
{
}

datafile_factory *
datafile_factory::instance()
{
    return new datafile_factory;
    // destraction will manage by ref_count installed in adplugin::plugin base class
}


void
datafile_factory::close( adcontrols::datafile * p )
{
    delete p;
}

const std::wstring&
datafile_factory::name() const
{
    static std::wstring name( L"text" );
    return name;
}

bool
datafile_factory::access( const std::wstring& filename, adcontrols::access_mode mode ) const
{
    boost::filesystem::wpath path(filename);
    return path.extension() == L".txt" && mode == adcontrols::read_access;
}

adcontrols::datafile *
datafile_factory::open( const std::wstring& filename, bool readonly ) const
{
    datafile * p = new datafile;
    if ( p->open( filename, readonly ) )
        return p;
    delete p;
    return 0;
}

////////////////////////////////////////////
// adplugin::plugin implementation

const char *
datafile_factory::iid() const 
{
    return "com.ms-cheminfo.qtplatz.plugins.datafile_factory";
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
