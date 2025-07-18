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
#include "datareader_factory.hpp"
#include "adplugin/plugin.hpp"
#include "datafile.hpp"
#include <adfs/sqlite.hpp>
#include <adplugin/visitor.hpp>
#include <adportable/debug.hpp>
#include <adutils/datafile_signature.hpp>
#include <mutex>

using namespace mzml;

std::shared_ptr< datafile_factory > datafile_factory::instance_( 0 );

datafile_factory::~datafile_factory(void)
{
}

datafile_factory::datafile_factory()
{
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

const char *
datafile_factory::mimeTypes() const
{
	return
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\
<mime-info xmlns='http://www.freedesktop.org/standards/shared-mime-info'>\n\
  <mime-type type=\"application/xml\">\n\
    <sub-class-of type=\"application/octet-stream\"/>\n\
    <glob pattern=\"*.mzML\"/>\n                        \
  </mime-type>\n\
  </mime-info>\n";
}

const wchar_t *
datafile_factory::name() const
{
    return L"mzML";
}

bool
datafile_factory::access( const wchar_t * filename, adcontrols::access_mode mode ) const
{
    return access (std::filesystem::path{filename}, mode );
}

bool
datafile_factory::access( const std::filesystem::path& path, adcontrols::access_mode mode ) const
{
    if ( path.extension() == ".adfs" ) {
        adfs::sqlite db;
        if ( db.open( path.string().c_str() ) ) {
            if ( auto value = adutils::data_signature::find( db, "datafile_factory" ) ) {
                return adutils::data_signature::to_string( *value ) == std::string( this->iid() );
            }
        }
    }
	return ( ( path.extension() == L".mzML" || path.extension() == L".mzml" ) && ( mode == adcontrols::read_access ) );
}

adcontrols::datafile *
datafile_factory::open( const wchar_t * filename, bool readonly ) const
{
    return open( std::filesystem::path{ filename }, readonly );
}

adcontrols::datafile *
datafile_factory::open( const std::filesystem::path& path, bool readonly ) const
{
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
    return "com.ms-cheminfo.qtplatz.adplugins.datafile_factory.admzml";
}

void
datafile_factory::accept( adplugin::visitor& v, const char * adplugin )
{
    // support subsidery plugin install via v.visit
    datareader_factory::instance()->accept( v, adplugin );
}

void *
datafile_factory::query_interface_workaround( const char * typenam )
{
    if ( std::string( typenam ) == typeid( adcontrols::datafile_factory ).name() )
        return static_cast< adcontrols::datafile_factory *>(this);
    return 0;
}
