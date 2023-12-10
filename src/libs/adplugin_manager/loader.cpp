/**************************************************************************
** Copyright (C) 2013-2023 MS-Cheminformatics LLC
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
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
#include <adportable/scoped_debug.hpp>
#include <adplugin/constants.hpp>
#include <adplugin/plugin.hpp>
#include <boost/dll/shared_library.hpp>
#include <adlog/logger.hpp>
#include <boost/format.hpp>
#include <boost/exception/all.hpp>
#include <boost/version.hpp>
#include <boost/dll.hpp>
#include <boost/dll/import.hpp>
#include <regex>

using namespace adplugin;

namespace {
#if defined _DEBUG || defined DEBUG || !defined NDEBUG
# if defined WIN32
    constexpr static const char * const debug_trail = "d";
    static const std::filesystem::path __prefix( "" );
# elif defined __MACH__ || __APPLE__
    constexpr static const char * const debug_trail = "_debug";
    static const std::filesystem::path __prefix( "lib" );
# else
    constexpr static const char * const debug_trail = "";
    static const std::filesystem::path __prefix( "lib" );
# endif

#else // NDEBUG

    constexpr static const char * const debug_trail = "";
# if defined WIN32
    static const std::filesystem::path __prefix( "" );
# elif defined __MACH__ || __APPLE__
    static const std::filesystem::path __prefix( "lib" );
# else
    static const std::filesystem::path __prefix( "lib" );
# endif
#endif
} // namespace


std::string
loader::debug_suffix()
{
    return debug_trail;
}

std::filesystem::path
loader::shared_directory()
{
    // program_location: /opt/qtplatz/bin/qtplatz
    return boost::dll::program_location().parent_path().parent_path() / std::string( sharedDirectory ); // /opt/qtplatz/ lib/qtplatz
}

std::filesystem::path
loader::plugin_directory()
{
    return boost::dll::program_location().parent_path().parent_path() / std::string( pluginDirectory );
}

void
loader::populate( const std::filesystem::path& appdir )
{
    // ScopedDebug(__t);
    // __t << " : " << appdir;

#if defined __APPLE__
    std::filesystem::path modules(    appdir / pluginDirectory ); // apple: Contents/PlugIns
    std::filesystem::path sharedlibs( appdir / sharedDirectory ); // apple: Contents/Frameworks
#else
    // search all files under ./lib/ directory
    std::filesystem::path modules(    appdir / "lib" ); // /qtplatz/plugins" );
#endif

    if ( std::filesystem::is_directory( modules ) ) {
        std::error_code ec;
        std::filesystem::recursive_directory_iterator it( modules, ec );
        if ( !ec ) {
            while ( it != std::filesystem::recursive_directory_iterator() ) {
                if ( std::filesystem::is_regular_file( it->status() ) ) {
                    // attempt to find a newly defined interface as of 2023-SEP-02
                    if ( it->path().extension() == boost::dll::shared_library::suffix() )  {
                        if ( it->path().string().find( "libadnetcdf" ) != std::string::npos ) {
                            try {
                                // ADDEBUG() << "\n\n-------- loading " << it->path();
                                auto instance = boost::dll::import_alias< adplugin::plugin *() >( it->path(), "adplugin_instance" );
                                if ( manager::instance()->install( boost::dll::shared_library( it->path() ), instance ) ) {
#ifndef NDEBUG
                                    ADDEBUG() << "---- load\t" << std::filesystem::relative( it->path(), appdir ) << "\tSuccess";
#endif
                                }
                            } catch ( std::exception& ex ) {
                                ADDEBUG() << "Exception:" << ex.what() << "\n\n";
                            }
                        }
                    }

                    auto filename = it->path().filename().string();
                    if ( it->path().extension() == L".adplugin" && !manager::instance()->isLoaded( it->path().string() ) ) {
                        auto stem   = std::filesystem::path( it->path().stem().string() + debug_trail );
                        auto libname  = it->path().parent_path() / (stem.string() + debug_trail);
                        //
                        auto fullpath = it->path().parent_path() /
                            std::filesystem::path( __prefix.string() + stem.string() ).replace_extension(
                                boost::dll::shared_library::suffix() );

                        std::error_code ec;
                        boost::dll::shared_library dll( libname, ec, boost::dll::load_mode::append_decorations );
                        if ( !ec ) {
                            if ( dll.has( "adplugin_plugin_instance" ) ) {
                                auto factory = dll.get< adplugin::plugin *() >( "adplugin_plugin_instance" );
                                if ( auto plugin = factory() ) {
                                    if ( manager::instance()->install( std::move( dll ), it->path().generic_string() ) ) {
#ifndef NDEBUG
                                        ADDEBUG() << "load\t" << std::filesystem::relative( dll.location(), appdir ) << "\tSuccess";
#endif
                                    }
                                }
                            } else {
                                ADDEBUG() << "library\t" << std::filesystem::relative( dll.location(), appdir )
                                          << " has no interface. Load failed.";
                            }
                        } else {
                            ADDEBUG() << "## failed to load " << std::filesystem::relative( libname, appdir ) << "\t: " << ec.message();
                            ADDEBUG() << "\t--> " << std::filesystem::relative( fullpath, appdir ) << "\texists: "
                                      << std::filesystem::exists( fullpath );
                        }
                    }
                }
                ++it;
            }
        }
    } else {
        ADDEBUG() << boost::format( "## Error: loader %1% is not a directory" ) % modules.generic_string();
#ifdef NDEBUG
        BOOST_THROW_EXCEPTION( std::runtime_error( ( boost::format( "loader %1% is not directory" ) % modules.generic_string() ).str() ) );
#endif
    }
}

// static
std::string
loader::library_filename( const char * library )
{
    std::string dname( library );
    dname += debug_trail;
    return dname;
}

// static
std::wstring
loader::config_fullpath( const std::wstring& apppath, const std::wstring& library_filename )
{
	std::filesystem::path path = std::filesystem::path( apppath ) / pluginDirectory;
	std::filesystem::path fullpath = path / library_filename;
	return fullpath.generic_wstring();
}

adplugin::plugin *
loader::loadLibrary( const std::string& stem )
{
    std::error_code ec;
    if ( auto dll = loadLibrary( stem, ec ) ) {
        if ( auto factory = dll.get< adplugin::plugin *() >( "adplugin_plugin_instance" ) ) {
            manager::instance()->keep( dll );
            return factory();
        }
    }

    // ADDEBUG() << "### Warning: loadLibrary(" << stem << ") failed: " << ec.message();
    return nullptr;
}

boost::dll::shared_library
loader::loadLibrary( const std::string& stem, std::error_code& ec )
{
    auto cdir = std::filesystem::canonical( boost::dll::this_line_location() ).parent_path();
    auto slib = cdir.parent_path() / sharedDirectory;

    // loading library from shared directory, which is as same as native qtplatz lib (./libs/qtplatz/)

    // ADDEBUG() << "=====> loadLibrary: cdir = " << cdir;
    // ADDEBUG() << "=====> loadLibrary(" << stem << ")\t" << slib;
    // ADDEBUG() << "\t" << ( slib / ( stem + debug_suffix() ) );

    //loader.cpp(194): =====> loadLibrary: cdir =   "/home/toshi/src/build-Linux-x86_64/qtplatz5.release/qtplatz/lib/qtplatz"
    //loader.cpp(195): =====> loadLibrary(u5303a)	"/home/toshi/src/build-Linux-x86_64/qtplatz5.release/qtplatz/lib/lib/qtplatz"
    //loader.cpp(196):                              "/home/toshi/src/build-Linux-x86_64/qtplatz5.release/qtplatz/lib/lib/qtplatz/u5303a"

    for ( auto p: { cdir, slib } ) {
        if ( auto dll = boost::dll::shared_library( p / ( stem + debug_suffix() )
                                                    , boost::dll::load_mode::append_decorations, ec ) )
            return dll;
    }
    return {};
}
