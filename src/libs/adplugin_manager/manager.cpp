/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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
#include <acewrapper/constants.hpp>
#include <adcontrols/datafile_factory.hpp>
#include <adcontrols/datafilebroker.hpp>
#include <adcontrols/massspectrometer_factory.hpp>
#include <adcontrols/massspectrometerbroker.hpp>
#include <adlog/logger.hpp>
#include <adlog/logging_handler.hpp>
#include <adplugin/adplugin.hpp>
#include <adplugin/constants.hpp>
#include <adplugin/lifecycle.hpp>
#include <adplugin/plugin.hpp>
#include <adplugin/plugin_ptr.hpp>
#include <adplugin/visitor.hpp>
#include <adportable/configloader.hpp>
#include <adportable/configuration.hpp>
#include <adportable/debug.hpp>
#include <adportable/debug_core.hpp>
#include <adportable/string.hpp>
#include <boost/cast.hpp>
#include <boost/dll.hpp>
#include <boost/exception/all.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <fstream>
#include <set>
#include <mutex>

#if defined _MSC_VER // std::regex on VS2012 got clash due to no instance of _CType.table depend on the timing
#define BOOST_REGEX
#endif

#if defined BOOST_REGEX
#include <boost/regex.hpp>
#else
#include <regex>
#endif

#include <algorithm>
#include <fstream>

//----------------------------------

using namespace adplugin;

////////////////

namespace adplugin {

    class plugin_data {
        adplugin::plugin_ptr plugin_;
        boost::dll::shared_library dll_;
        plugin_data( const plugin_data& t ) = delete;
        const plugin_data& operator = ( const plugin_data& t ) = delete;
    public:
        ~plugin_data() {
            plugin_ = nullptr; // release plugin before unload library
#ifndef NDEBUG
            auto loc = boost::filesystem::relative( dll_.location(), boost::dll::program_location().parent_path() );
#endif
            dll_.unload();
#ifndef NDEBUG
            ADDEBUG() << "<<< unloading " << loc << " " << (dll_.is_loaded() ? " fail" : " success");
#endif
        }

        plugin_data() {
        }

        plugin_data( adplugin::plugin_ptr ptr ) : plugin_( ptr ) {
        }

        explicit plugin_data( adplugin::plugin_ptr ptr
                              , boost::dll::shared_library&& dll ) : plugin_( ptr )
                                                                   , dll_( dll ) {
#if !defined NDEBUG && 0
            boost::system::error_code ec;
            ADDEBUG() << ">>> plugin_data ctor : " << dll_.location( ec );
#endif
        }

        const char * clsid() const {
            return plugin_->clsid();
        }

        const char * iid() const {
            return plugin_->iid();
        }

        adplugin::plugin_ptr plugin() const { return plugin_; }

        adplugin::plugin * p() { return plugin_.get(); }

        boost::filesystem::path path() const {
            return dll_.location();
        }

        bool operator == ( const adplugin::plugin& t ) const {
            return plugin_.get() == &t; // equal address
        }
    };

    class manager::impl : adplugin::visitor {

        impl( const impl& ) = delete;
        impl& operator = ( const impl ) = delete;

    public:
        virtual ~impl() {
            // clear all factories before unload library
            adcontrols::MassSpectrometerBroker::clear_factories();
            adcontrols::datafileBroker::clear_factories();
            this->keeper_.clear();
        }
        impl() {}

        typedef std::vector< adplugin::plugin_ptr > vector_type;

        bool install( boost::dll::shared_library&& lib, const std::string& adpluginspec, const std::string& context ) {
            if ( pluginspecs_.find( adpluginspec ) != pluginspecs_.end() )
                return false;
            if ( lib.has( "adplugin_plugin_instance" ) ) {
                if ( auto factory = lib.get< adplugin::plugin *() >( "adplugin_plugin_instance" ) ) {
                    if ( auto plugin = factory() ) {
                        pluginspecs_.insert( adpluginspec );
                        plugin->setConfig( adpluginspec, context, lib.location().string() );
                        plugins_.emplace( plugins_.begin(), std::make_shared< plugin_data >( plugin->pThis(), std::move( lib ) ) ); // reverse order
                        plugin->accept( *this, adpluginspec.c_str() );
                        return true;
                    }
                }
            }
            return false;
        }

        bool install( boost::dll::shared_library&& dll, std::function<adplugin::plugin *()> instance ) {
            if ( auto plugin = instance() ) {
                ADDEBUG() << "2023-09-02 interface: " << plugin->iid() << ", is_loaded: " << dll.is_loaded();
                //pluginspecs_.insert( adpluginspec );
                //plugin->setConfig( adpluginspec, context, lib.location().string() );
                plugins_.emplace( plugins_.begin(), std::make_shared< plugin_data >( plugin->pThis(), std::move( dll ) ) ); // reverse order
                plugin->accept( *this, "" );
                return true;
            }
            return false;
        }

        void populated();

        plugin_ptr select_iid( const char * regex );
        size_t select_iids( const char * regex, std::vector< plugin_ptr >& );

        size_t select_plugins( const char * regex, std::vector< plugin_ptr >& ); // added 2016-07-05, for mpxdatainterpreter

        bool isLoaded( const std::string& adpluginspec ) const {
            return pluginspecs_.find( adpluginspec ) != pluginspecs_.end();
        }

        // visitor
        // allow additional (subsidary) pulugin install
        void visit( adplugin::plugin * plugin, const char * adpluginspec ) {
            if ( adpluginspec == 0 || plugin == 0 )
                return;
            // make it unique
            auto it = std::find_if( plugins_.begin(), plugins_.end(), [&](const auto& d){ return *d == (*plugin); });
            if ( it == plugins_.end() )
                additionals_.emplace_back( plugin->pThis() );
        }

    private:
        std::set< std::string > pluginspecs_;
        std::vector< std::shared_ptr< plugin_data > > plugins_;
        vector_type additionals_; // if shared-object contains more than two plugins
        std::mutex mutex_;
    public:
        std::vector< boost::dll::shared_library > keeper_; // avoiding unload
    };
}

////////////////////////////////////

manager *
manager::instance()
{
    static manager __manager__;
    return &__manager__;
}

//////////////////////

manager::manager(void) : d_( new manager::impl() )
{
}

manager::~manager(void)
{
    delete d_;
#ifndef NDEBUG
    ADDEBUG() << "<<< manager dtor completed.";
#endif
}

bool
manager::install( boost::dll::shared_library&& dll, const std::string& adpluginspec )
{
    std::ostringstream s;
    std::ifstream inf( adpluginspec.c_str() );

    // read contents of .adplugin file
    std::copy( std::istreambuf_iterator<char>(inf), std::istreambuf_iterator<char>(), std::ostreambuf_iterator<char>(s) );

    return d_->install( std::move( dll ), adpluginspec, s.str() );
}

bool
manager::install( boost::dll::shared_library&& dll, std::function<adplugin::plugin *()> instance )
{
    return d_->install( std::move( dll ), std::move( instance ) );
}


bool
manager::isLoaded( const std::string& adpluginspec ) const
{
    return d_->isLoaded( adpluginspec );
}

plugin_ptr
manager::select_iid( const char * regex )
{
    return d_->select_iid( regex );
}

size_t
manager::select_iids( const char * regex, std::vector< plugin_ptr >& vec )
{
    return d_->select_iids( regex, vec );
}

std::vector< plugin_ptr >
manager::select_plugins( const char * regex )
{
    std::vector< plugin_ptr > v;

    d_->select_plugins( regex, v );

    return v;
}


plugin_ptr
manager::impl::select_iid( const char * regex )
{
#if defined BOOST_REGEX
	boost::regex re( regex );
    boost::cmatch matches;
    using namespace boost;
#else
	std::regex re( regex );
    std::cmatch matches;
    using namespace std;
#endif

	auto it = std::find_if( plugins_.begin(), plugins_.end(), [&]( const auto& p ){ return regex_match( p->iid(), matches, re ); });
	if ( it != plugins_.end() )
		return (*it)->plugin();
    return 0;
}

size_t
manager::impl::select_iids( const char * regex, std::vector< plugin_ptr >& vec )
{
#ifdef BOOST_REGEX
	boost::regex re( regex );
	boost::cmatch matches;
    using namespace boost;
#else
	std::regex re( regex );
	std::cmatch matches;
    using namespace std;
#endif

	std::for_each( plugins_.begin(), plugins_.end()
                   , [&]( const auto& p ){
                         if ( regex_match( p->iid(), matches, re ) )
                             vec.emplace_back( p->plugin() );
                     } );
    return vec.size();
}

size_t
manager::impl::select_plugins( const char * regex, std::vector< plugin_ptr >& vec )
{
#ifdef BOOST_REGEX
	boost::regex re( regex );
	boost::cmatch matches;
    using namespace boost;
#else
	std::regex re( regex );
	std::cmatch matches;
    using namespace std;
#endif

	std::for_each( plugins_.begin(), plugins_.end()
                   , [&]( const auto& m ){
                         if ( regex_match( m->path().string().c_str(), matches, re ) )
                             vec.emplace_back( m->plugin() );
		} );
    return vec.size();
}


//static
void
manager::standalone_initialize()
{
#if defined WIN32
    // Windows -->   qtplatz.rekiease/bin/adplugin_manager.dll
    auto tpath = boost::dll::this_line_location().parent_path().parent_path();
#elif defined __APPLE__
    // macOS --> ./bin/qtplatz.app/Contents/Frameworks/libadplugin_manager.dylib
    auto tpath = boost::dll::this_line_location().parent_path().parent_path().parent_path();
#else
    // Linux --> ./qtplatz/lib/qtplatz/libadplugin_manager.so|./qtplatz/lib/libadplugin_manager.so
    // Linux --> ./qtplatz/bin/qtplatz
    auto tpath = boost::dll::program_location().parent_path().parent_path();  // <-- install dir (/opt/qtplatz/bin/qtplatz/../..)
#endif

    adplugin::loader::populate( tpath );

    // spectrometers
	std::vector< adplugin::plugin_ptr > spectrometers;
	if ( adplugin::manager::instance()->select_iids( ".*\\.adplugins\\.massSpectrometer\\..*", spectrometers ) ) {
		std::for_each( spectrometers.begin()
                       , spectrometers.end()
                       , []( const adplugin::plugin_ptr& d ){
                             if ( auto factory = d->query_interface< adcontrols::massspectrometer_factory >() )
                                 adcontrols::MassSpectrometerBroker::register_factory( factory );
                         });
	}

    // dataproverders
    std::vector< adplugin::plugin_ptr > dataproviders;
    std::vector< std::string > mime; // todo

    if ( adplugin::manager::instance()->select_iids( ".*\\.adplugins\\.datafile_factory\\..*", dataproviders ) ) {

        std::for_each( dataproviders.begin()
                       , dataproviders.end()
                       , [&] ( const adplugin::plugin_ptr& d ) {
                             if ( auto factory = d->query_interface< adcontrols::datafile_factory >() ) {
                                 adcontrols::datafileBroker::register_factory( factory, d->clsid() );
                                 if ( factory->mimeTypes() )
                                     mime.emplace_back( factory->mimeTypes() );
                             }
                         });
    }
}

void
manager::keep( const boost::dll::shared_library& dll )
{
    d_->keeper_.emplace_back( dll );
}
