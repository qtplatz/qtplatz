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
#include <QCoreApplication>
#include <QFileInfo>
#include <QLibrary>
#include <QString>
#include <QStringList>
#include <boost/format.hpp>
#include <boost/exception/all.hpp>

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
loader::populate( const wchar_t * topdir )
{
    boost::filesystem::path appdir( topdir );
    boost::filesystem::path modules( appdir / pluginDirectory );
    boost::filesystem::path sharedlibs( appdir / sharedDirectory );

    ADDEBUG() << "loader populating : " << topdir;

    if ( boost::filesystem::is_directory( modules ) ) {

        boost::system::error_code ec;
        boost::filesystem::recursive_directory_iterator it( modules, ec );

        if ( !ec ) {

            while ( it != boost::filesystem::recursive_directory_iterator() ) {

                if ( boost::filesystem::is_regular_file( it->status() ) ) {

                    if ( it->path().extension() == L".adplugin" && !manager::instance()->isLoaded( it->path().string() ) ) {

                        auto stem = it->path().stem();
                        auto branch = it->path().branch_path();

                        for ( auto& dir : { branch, sharedlibs } ) {
                            QString libname = QString::fromStdString( ( dir / stem ).string() + DEBUG_LIB_TRAIL );
                            QLibrary lib( libname );

                            ADDEBUG() << "\tloading : " << libname.toStdString();
                                                    
                            if ( lib.load() && manager::instance()->install( lib, it->path().generic_string() ) ) {
                                break;
                            } else {
#if defined __APPLE__
                                // somewhat 'DEBUG' can't define on apple with Xcode
                                QLibrary lib( libname + "_debug" );
                                if ( lib.load() && manager::instance()->install( lib, it->path().generic_string() ) )
                                    break;
#endif
                                ADDEBUG() << "## failed to load: " << libname.toStdString()
                                          << "\n\t" << lib.errorString().toStdString();
                            }
                        }
                    }
                }
                ++it;
            }
        }
    } else {
        BOOST_THROW_EXCEPTION( std::runtime_error( ( boost::format( "loader %1% is not directory" ) % modules.generic_string() ).str() ) );
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

#if 0
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
#endif

// static
std::wstring
loader::config_fullpath( const std::wstring& apppath, const std::wstring& library_filename )
{
	boost::filesystem::path path = boost::filesystem::path( apppath ) / pluginDirectory;
	boost::filesystem::path fullpath = path / library_filename; 
	return fullpath.generic_wstring();
}

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
