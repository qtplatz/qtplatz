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
#include <boost/format.hpp>
#include <boost/exception/all.hpp>
#include <boost/version.hpp>
#include <boost/dll.hpp>

using namespace adplugin;

#if defined _DEBUG || defined DEBUG || !defined NDEBUG
# if defined WIN32
#  define DEBUG_LIB_TRAIL "d" // xyzd.dll
constexpr static const char * const debug_trail = "d";
# elif defined __MACH__ || __APPLE__
#  define DEBUG_LIB_TRAIL "_debug" // xyz_debug.dylib
constexpr static const char * const debug_trail = "_debug";
# else
#  define DEBUG_LIB_TRAIL ""        // xyz.so
constexpr static const char * const debug_trail = "";
# endif
#else
# define DEBUG_LIB_TRAIL ""
constexpr static const char * const debug_trail = "";
#endif

std::string
loader::debug_suffix()
{
    return debug_trail;
}

boost::filesystem::path
loader::shared_directory()
{
    return boost::dll::program_location().parent_path() / std::string( sharedDirectory );
}

boost::filesystem::path
loader::plugin_directory()
{
    return boost::dll::program_location().parent_path() / std::string( pluginDirectory );
}

void
loader::populate( const wchar_t * topdir )
{
    boost::filesystem::path appdir( topdir );
    boost::filesystem::path modules( appdir / pluginDirectory );
    boost::filesystem::path sharedlibs( appdir / sharedDirectory );
#ifndef NDEBUG
    ADDEBUG() << "loader populating in directory: " << topdir;
#endif
    if ( boost::filesystem::is_directory( modules ) ) {

        boost::system::error_code ec;
        boost::filesystem::recursive_directory_iterator it( modules, ec );

        if ( !ec ) {

            while ( it != boost::filesystem::recursive_directory_iterator() ) {

                if ( boost::filesystem::is_regular_file( it->status() ) ) {

                    if ( it->path().extension() == L".adplugin" && !manager::instance()->isLoaded( it->path().string() ) ) {

                        auto stem = it->path().stem();
                        auto branch = it->path().branch_path();
                        
                        for ( auto& dir : { branch /*, sharedlibs */ } ) {

                            auto fname = dir / (stem.string() + debug_trail);
                            boost::system::error_code ec;
                            boost::dll::shared_library dll( fname, boost::dll::load_mode::append_decorations, ec );
                            if ( dll && manager::instance()->install( std::move( dll ), it->path().generic_string() ) ) {
                                ADDEBUG() << "loading\n\t" << dll.location() << "\tSuccess";
                                break;
                            } else {
                                ADDEBUG() << "loading\n\t" << fname << "\t" << ec.message();
                            }
                        }
                    }
                }
                ++it;
            }
        }
    } else {
        ADDEBUG() << boost::format( "## Error: loader %1% is not a directory" ) % modules.generic_string();
        BOOST_THROW_EXCEPTION( std::runtime_error( ( boost::format( "loader %1% is not directory" ) % modules.generic_string() ).str() ) );
    }
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

// static
std::wstring
loader::config_fullpath( const std::wstring& apppath, const std::wstring& library_filename )
{
	boost::filesystem::path path = boost::filesystem::path( apppath ) / pluginDirectory;
	boost::filesystem::path fullpath = path / library_filename;
	return fullpath.generic_wstring();
}

adplugin::plugin *
loader::loadLibrary( const std::string& stem )
{
    boost::system::error_code ec;
    if ( auto dll = loadLibrary( stem, ec ) ) {
        if ( auto factory = dll.get< adplugin::plugin *() >( "adplugin_plugin_instance" ) ) {
            manager::instance()->keep( dll );
            return factory();
        }
    }
    return nullptr;
}

boost::dll::shared_library
loader::loadLibrary( const std::string& stem, boost::system::error_code& ec )
{
    auto path = adplugin::loader::shared_directory() / ( stem + debug_suffix() );
    return boost::dll::shared_library( path, boost::dll::load_mode::append_decorations, ec );
}


#if 0
adplugin::plugin *
loader::loadLibrary( const QString& libname, const QStringList& paths )
{
    auto appdir = QCoreApplication::applicationDirPath(); // ~/Applications/qtplatz/bin
    boost::filesystem::path install_path( boost::filesystem::path( appdir.toStdWString() ).parent_path() );

    boost::filesystem::path stem( ( libname + DEBUG_LIB_TRAIL ).toStdWString() );

    QLibrary lib;
    typedef adplugin::plugin * ( *factory )();

    for ( auto& path : paths ) {
        boost::filesystem::path p( path.toStdWString() );
        if ( p.is_absolute() )
            lib.setFileName( QString::fromStdWString( ( p / stem ).wstring() ) );
        else
            lib.setFileName( QString::fromStdWString( ( install_path / p / stem ).wstring() ) );
        if ( lib.load() || lib.isLoaded() ) {
            if ( factory f = reinterpret_cast<factory>( lib.resolve( "adplugin_plugin_instance" ) ) )
                return f();
        }
    }

    boost::filesystem::path sharedpath( install_path / sharedDirectory );
    lib.setFileName( QString::fromStdWString( ( sharedpath / stem ).wstring() ) );
    if ( lib.load() || lib.isLoaded() ) {
        if ( factory f = reinterpret_cast<factory>( lib.resolve( "adplugin_plugin_instance" ) ) )
            return f();
    }


    boost::filesystem::path modulepath( install_path / pluginDirectory );
    lib.setFileName( QString::fromStdWString( ( sharedpath / stem ).wstring() ) );
    if ( lib.load() || lib.isLoaded() ) {
        if ( factory f = reinterpret_cast<factory>( lib.resolve( "adplugin_plugin_instance" ) ) )
            return f();
    }

    boost::filesystem::recursive_directory_iterator it( install_path / pluginDirectory );

    while ( it != boost::filesystem::recursive_directory_iterator() ) {
        if ( boost::filesystem::is_directory( it->status() ) ) {
            lib.setFileName( QString::fromStdWString( ( it->path() / stem ).wstring() ) );
            if ( lib.load() || lib.isLoaded() ) {
                if ( factory f = reinterpret_cast<factory>( lib.resolve( "adplugin_plugin_instance" ) ) )
                    return f();
            }
        }
        ++it;
    }

    lib.setFileName( QString::fromStdWString( stem.wstring() ) );
    if ( lib.load() || lib.isLoaded() ) {
        if ( factory f = reinterpret_cast<factory>( lib.resolve( "adplugin_plugin_instance" ) ) )
            return f();
    }
    return 0;
}
#endif
