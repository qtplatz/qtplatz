// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include <compiler/disable_unused_parameter.h>

#include "datafile_factory.hpp"
#include "datafile.hpp"
#include <adplugin/plugin.hpp>
#include <adplugin/visitor.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <mutex>

using namespace addatafile;

datafile_factory * datafile_factory::instance_ = 0;
std::mutex __mutex;

datafile_factory::~datafile_factory(void)
{
}

datafile_factory::datafile_factory()
{
	instance_ = 0;
}

datafile_factory *
datafile_factory::instance()
{
    if ( instance_ == 0 ) {
        std::lock_guard< std::mutex > lock( __mutex );
        if ( instance_ == 0 ) {
            instance_ = new datafile_factory;
			// destractor will call from adplugin::dispose by reference couting method,
            // so 'singleton' mechanism may not be necessary though...
		}
    }
    return instance_;
}

void
datafile_factory::close( adcontrols::datafile * p )
{
    delete p;
}

const wchar_t *
datafile_factory::name() const
{
	return L"addatafile";
}

bool
datafile_factory::access( const wchar_t * filename, adcontrols::access_mode mode ) const
{
    boost::filesystem::wpath path(filename);
    if ( path.extension() == L".qtms" ) // obsolete
        return mode == adcontrols::read_access;
    if ( path.extension() == L".adfs" )
        return mode == adcontrols::read_access || mode == adcontrols::write_access;
    return false;
}

adcontrols::datafile *
datafile_factory::open( const wchar_t * filename, bool readonly ) const
{
    boost::filesystem::wpath path(filename);
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
    return "com.ms-cheminfo.qtplatz.adplugins.datafile_factory.addatafile";
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
