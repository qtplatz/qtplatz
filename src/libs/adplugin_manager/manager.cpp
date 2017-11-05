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

#include <compiler/disable_unused_parameter.h>
#include "manager.hpp"
#include "loader.hpp"
#include <adcontrols/datafile_factory.hpp>
#include <adcontrols/datafilebroker.hpp>
#include <adcontrols/massspectrometer_factory.hpp>
#include <adcontrols/massspectrometerbroker.hpp>
#include <adplugin/adplugin.hpp>
#include <adplugin/lifecycle.hpp>
#include <adplugin/constants.hpp>
#include <adplugin/plugin.hpp>
#include <adplugin/plugin_ptr.hpp>
#include <adplugin/visitor.hpp>
#include "loader.hpp"
#include <adportable/configuration.hpp>
#include <acewrapper/constants.hpp>
#include <adportable/configloader.hpp>
#include <adportable/debug.hpp>
#include <adportable/debug_core.hpp>
#include <adlog/logger.hpp>
#include <adlog/logging_handler.hpp>
#include <adportable/string.hpp>
#include <qtwrapper/qstring.hpp>
#include <QCoreApplication>
#include <QLibrary>
#include <fstream>
#include <map>
#include <mutex>

#include <compiler/diagnostic_push.h>
#include <compiler/disable_unused_variable.h>
#include <boost/filesystem.hpp>
#include <compiler/diagnostic_pop.h>
#include <boost/cast.hpp>
#include <boost/exception/all.hpp>
#include <boost/filesystem/fstream.hpp>

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
    public:
        plugin_data() {
        }

        plugin_data( adplugin::plugin_ptr ptr ) : plugin_( ptr ) {
        }

        plugin_data( const plugin_data& t ) : plugin_( t.plugin_ ) {
        }

        const char * clsid() const { 
            return plugin_->clsid();
        }

        const char * iid() const { 
            return plugin_->iid(); 
        }

        adplugin::plugin_ptr plugin() const { return plugin_; }

        adplugin::plugin * p() { return plugin_.get(); }

        bool operator == ( const adplugin::plugin& t ) const {
            if ( plugin_.get() == &t ) // equal address
                return true;
            return ( plugin_->clsid() == t.clsid() &&  plugin_->iid() == t.iid() );
        }

    };

    class manager::data : adplugin::visitor {

        data( const data& ) = delete;
        data& operator = ( const data& ) = delete;

    public:
        virtual ~data() {}
        data() {}
        
        typedef std::map< std::string, plugin_data > map_type;
        typedef std::vector< plugin_data > vector_type;

        bool install( QLibrary&, const std::string& adpluginspec, const std::string& context );

        void populated();

        plugin_ptr select_iid( const char * regex );
        size_t select_iids( const char * regex, std::vector< plugin_ptr >& );

        size_t select_plugins( const char * regex, std::vector< plugin_ptr >& ); // added 2016-07-05, for mpxdatainterpreter 

        bool isLoaded( const std::string& adpluginspec ) const;
        
        // visitor 
        void visit( adplugin::plugin * plugin, const char * adpluginspec );
        
    private:
        map_type plugins_;
        vector_type additionals_; // if shared-object contains more than two plugins
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

manager::manager(void) : d_( new manager::data() )
{
}

manager::~manager(void)
{
    delete d_;
}

bool
manager::install( QLibrary& lib, const std::string& adpluginspec )
{
    std::ostringstream s;
    std::ifstream inf( adpluginspec.c_str() );

    // read contents of .adplugin file
    std::copy( std::istreambuf_iterator<char>(inf), std::istreambuf_iterator<char>(), std::ostreambuf_iterator<char>(s) );

    return d_->install( lib, adpluginspec, s.str() );
}

bool
manager::isLoaded( const std::string& adpluginspec ) const
{
    return d_->isLoaded( adpluginspec );
}

void
manager::populated()
{
	return d_->populated();
}

plugin_ptr
manager::select_iid( const char * regex )
{
    return d_->select_iid( regex );
}

// plugin_ptr
// manager::select_clsid( const char * regex )
// {
//     return d_->select_clsid( regex );
// }

size_t
manager::select_iids( const char * regex, std::vector< plugin_ptr >& vec )
{
    return d_->select_iids( regex, vec );
}

// size_t
// manager::select_clsids( const char * regex, std::vector< plugin_ptr >& vec )
// {
//     return d_->select_clsids( regex, vec );
// }

std::vector< plugin_ptr >
manager::select_plugins( const char * regex )
{
    std::vector< plugin_ptr > v;

    d_->select_plugins( regex, v );

    return v;
}


//////////////////

void
manager::data::visit( adplugin::plugin * plugin, const char * adpluginspec )
{
    if ( adpluginspec == 0 || plugin == 0 )
        return;

    // make it unique
	auto it = std::find_if( plugins_.begin(), plugins_.end(), [&](const map_type::value_type& d){
            return d.second == (*plugin);
        });

	if ( it == plugins_.end() )
		additionals_.push_back( plugin_data( plugin->pThis() ) );
}

void
manager::data::populated()
{
#if ( defined _DEBUG || defined DEBUG ) && 0
    adportable::debug(__FILE__, __LINE__) << "==> populated";
	std::for_each( plugins_.begin(), plugins_.end(), [&](const map_type::value_type& d){
            adportable::debug(__FILE__, __LINE__) << "\t" << d.second.iid();
        });
    adportable::debug(__FILE__, __LINE__) << "<== populated";
#endif
}

bool
manager::data::isLoaded( const std::string& adpluginspec ) const
{
    return plugins_.find( adpluginspec ) != plugins_.end();
}

bool
manager::data::install( QLibrary& lib, const std::string& adpluginspec, const std::string& specxml )
{
    if ( plugins_.find( adpluginspec ) != plugins_.end() )
        return true; // already in, so that does not need unload() call

    typedef adplugin::plugin * ( *factory_type )();

    boost::filesystem::path path( lib.fileName().toStdString() );
    
    if ( auto factory = reinterpret_cast< factory_type >( lib.resolve( "adplugin_plugin_instance" ) ) ) {

        if ( adplugin::plugin * pptr = factory() ) {
            
            pptr->setConfig( adpluginspec, specxml, path.string() );
            plugins_[ adpluginspec ] = plugin_data( pptr->pThis() );
            pptr->accept( *this, adpluginspec.c_str() );
            
            return true;
        }

    }
    return false;
}

plugin_ptr
manager::data::select_iid( const char * regex )
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

	auto itr = std::find_if( plugins_.begin(), plugins_.end(), [&]( const map_type::value_type& m ){
            return regex_match( m.second.iid(), matches, re );
        });
	if ( itr != plugins_.end() )
		return itr->second.plugin();
    return 0;
}

size_t
manager::data::select_iids( const char * regex, std::vector< plugin_ptr >& vec )
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

	std::for_each( plugins_.begin(), plugins_.end(), [&]( const map_type::value_type& m ){
            if ( regex_match( m.second.iid(), matches, re ) )
                vec.push_back( m.second.plugin() );
		} );
    return vec.size();
}

size_t
manager::data::select_plugins( const char * regex, std::vector< plugin_ptr >& vec )
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

	std::for_each( plugins_.begin(), plugins_.end(), [&]( const map_type::value_type& m ){
            if ( regex_match( m.first.c_str(), matches, re ) )
                vec.push_back( m.second.plugin() );
		} );
    return vec.size();
}


//static
void
manager::standalone_initialize()
{
    auto apath = QCoreApplication::instance()->applicationDirPath().toStdWString();
    auto tpath = boost::filesystem::canonical( boost::filesystem::path( apath ) / "../" );

    adplugin::loader::populate( tpath.wstring().c_str() );

    // spectrometers
	std::vector< adplugin::plugin_ptr > spectrometers;
	if ( adplugin::manager::instance()->select_iids( ".*\\.adplugins\\.massSpectrometer\\..*", spectrometers ) ) {
		std::for_each( spectrometers.begin(), spectrometers.end(), []( const adplugin::plugin_ptr& d ){ 
                adcontrols::massspectrometer_factory * factory = d->query_interface< adcontrols::massspectrometer_factory >();
                if ( factory )
                    adcontrols::MassSpectrometerBroker::register_factory( factory );
            });
	}
    
    // dataproverders
    std::vector< adplugin::plugin_ptr > dataproviders;
    std::vector< std::string > mime; // todo

    if ( adplugin::manager::instance()->select_iids( ".*\\.adplugins\\.datafile_factory\\..*", dataproviders ) ) {
        
        std::for_each( dataproviders.begin(), dataproviders.end(), [&] ( const adplugin::plugin_ptr& d ) {
                adcontrols::datafile_factory * factory = d->query_interface< adcontrols::datafile_factory >();
                if ( factory ) {
                    ADDEBUG() << "installing " << factory->name() << "...";
                    adcontrols::datafileBroker::register_factory( factory, d->clsid() );
                    if ( factory->mimeTypes() )
                        mime.push_back( factory->mimeTypes() );
                }
            } );
    }
}
