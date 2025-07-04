// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2025 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2025 MS-Cheminformatics LLC
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
#include <adutils/datafile_signature.hpp>
#include <adportable/debug.hpp>
#include <adplugin/plugin.hpp>
#include <adplugin/visitor.hpp>
#include <adfs/sqlite.hpp>
#include <filesystem>
#include <boost/config.hpp>
#include <boost/dll/alias.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <mutex>

using namespace addatafile;

std::shared_ptr< datafile_factory > datafile_factory::instance_ = 0;

datafile_factory::~datafile_factory(void)
{
}

datafile_factory::datafile_factory()
{
	instance_ = 0;
}

//datafile_factory *
adplugin::plugin *
datafile_factory::instance()
{
    static std::once_flag flag;
    std::call_once( flag, [] () { instance_ = std::make_shared< datafile_factory >(); } );
    return instance_.get();
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
    return access( std::filesystem::path{ filename }, mode );
}

bool
datafile_factory::access( const std::filesystem::path& path, adcontrols::access_mode mode ) const
{
    if ( path.extension() == ".qtms" )  // obsolete
        return mode == adcontrols::read_access;

    if ( path.extension() == ".adfs" || path.extension() == ".adfs~" ) {
        adfs::sqlite db;
        if ( db.open( path.string().c_str() ) ) {
            using namespace adutils::data_signature;
            if ( auto value = find( db, "datafile_factory" ) ) {
                // return to_string( *value ) == std::string( this->iid() );
                ADDEBUG() << "datafile_factory: " << std::make_pair(to_string( *value ), std::string( this->iid() ));
                return true; // workaround until mzML imported data handling to be done.
            }
        }
        return mode == adcontrols::read_access || mode == adcontrols::write_access;
    }
    return false;
}

adcontrols::datafile *
datafile_factory::open( const wchar_t * filename, bool readonly ) const
{
    return open( std::filesystem::path{ filename }, readonly );
}

adcontrols::datafile *
datafile_factory::open( const std::filesystem::path& path, bool readonly ) const
{
    // ADDEBUG() << "===> datafile_factory::open(" << filename << ")";
    datafile * p = new datafile;
    if ( p->open( path.wstring(), readonly ) )
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
    // no need to call visitor due to no additional plugin
	// v.visit( this, adplugin );
}

void *
datafile_factory::query_interface_workaround( const char * typenam )
{
    if ( std::string( typenam ) == typeid( adcontrols::datafile_factory ).name() )
        return static_cast< adcontrols::datafile_factory *>(this);
    return 0;
}
