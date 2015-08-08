/**************************************************************************
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
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

#include "loader.hpp"
#include "manager.hpp"
#include <adportable/debug.hpp>
#include <adplugin/constants.hpp>
#include <boost/filesystem.hpp>
#include <adlog/logger.hpp>
#include <QLibrary>

using namespace adplugin;

#if defined _DEBUG || defined DEBUG
# if defined WIN32
#  define DEBUG_LIB_TRAIL "d" // xyzd.dll
# elif defined __MACH__
#  define DEBUG_LIB_TRAIL "_debug" // xyz_debug.dylib
# else
#  define DEBUG_LIB_TRAIL ""        // xyz.so 
# endif
#else
# define DEBUG_LIB_TRAIL ""
#endif

void
loader::populate( const wchar_t * directory )
{
    boost::filesystem::path appdir( directory );
    boost::filesystem::path modules( appdir / pluginDirectory );
    boost::filesystem::path sharedlibs( appdir / sharedDirectory );
    
    boost::filesystem::recursive_directory_iterator it( directory );

    while ( it != boost::filesystem::recursive_directory_iterator() ) {

        if ( boost::filesystem::is_regular_file( it->status() ) ) {
            if ( it->path().extension() == L".adplugin" ) {
                auto stem = it->path().stem();
                auto branch = it->path().branch_path();

                for ( auto& dir : { branch, sharedlibs } ) {
                    QString libname = QString::fromStdString( ( dir / stem ).string() + DEBUG_LIB_TRAIL );
                    QLibrary lib( libname );
                    if ( lib.load() && manager::instance()->install( lib, it->path().generic_string() ) )
                        break;
                    lib.unload();
                }
            }
        }
        ++it;
    }
	manager::instance()->populated();
}

// static
std::string
loader::library_filename( const char * library )
{
    std::string dname( library );
#if defined DEBUG || defined _DEBUG
    dname += DEBUG_LIB_TRAIL;
#endif
    return dname;
}

void
loader::load( const wchar_t * /* library_filename */)
{
}

void
loader::unload( const wchar_t * /* library_filename */)
{
}

plugin_ptr
loader::select_iid( const char * iid )
{
    return manager::instance()->select_iid( iid );
}

size_t
loader::select_iids( const char * regex, std::vector< plugin_ptr >& vec )
{
    return manager::instance()->select_iids( regex, vec );
}

plugin_ptr
loader::select_clsid( const char * clsid )
{
    return manager::instance()->select_clsid( clsid );
}

size_t
loader::select_clsids( const char * regex, std::vector< plugin_ptr >& vec )
{
    return manager::instance()->select_clsids( regex, vec );
}

// static
std::wstring
loader::config_fullpath( const std::wstring& apppath, const std::wstring& library_filename )
{
	boost::filesystem::path path = boost::filesystem::path( apppath ) / pluginDirectory;
	boost::filesystem::path fullpath = path / library_filename; 
	return fullpath.generic_wstring();
}
