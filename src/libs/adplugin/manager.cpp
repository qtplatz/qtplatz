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
#include "adplugin.hpp"
#include "lifecycle.hpp"
#include "constants.hpp"
#include "lifecycleaccessor.hpp"
#include "plugin.hpp"
#include "plugin_ptr.hpp"
#include "visitor.hpp"
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
#include <QLibrary>
#include <QDir>
#include <QMessageBox>
#include <QWidget>
#include <map>
#include <fstream>

#include <compiler/diagnostic_push.h>
#include <compiler/disable_unused_variable.h>
#include <boost/filesystem.hpp>
#include <compiler/diagnostic_pop.h>
#include <boost/filesystem/fstream.hpp>
#include <boost/cast.hpp>

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

    namespace internal {

        class manager_data : boost::noncopyable
                           , adplugin::visitor {
        public:
            virtual ~manager_data() {}
            manager_data() {}
            typedef std::map< std::string, plugin_data > map_type;
            typedef std::vector< plugin_data > vector_type;
            bool install( QLibrary&, const std::string& adpluginspec, const std::string& context );
			void populated();
            plugin_ptr select_iid( const char * regex );
            plugin_ptr select_clsid( const char * regex );
            size_t select_iids( const char * regex, std::vector< plugin_ptr >& );
            size_t select_clsids( const char * regex, std::vector< plugin_ptr >& );

            // visitor 
            void visit( adplugin::plugin * plugin, const char * adpluginspec );
        private:
            map_type plugins_;
            vector_type additionals_; // if shared-object contains more than two plugins
        };
    }

}
////////////////////////////////////

manager * manager::instance_ = 0;

manager *
manager::instance()
{
	if ( instance_ == 0 )
		instance_ = new manager;
	return instance_;
}

//////////////////////

manager::manager(void) : d_( new internal::manager_data() )
{
	adportable::core::debug_core::instance()->hook( adlog::logging_handler::log );
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
	std::copy( std::istreambuf_iterator<char>(inf), std::istreambuf_iterator<char>(), std::ostreambuf_iterator<char>(s) );
    return d_->install( lib, adpluginspec, s.str() );
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

plugin_ptr
manager::select_clsid( const char * regex )
{
    return d_->select_clsid( regex );
}

size_t
manager::select_iids( const char * regex, std::vector< plugin_ptr >& vec )
{
    return d_->select_iids( regex, vec );
}

size_t
manager::select_clsids( const char * regex, std::vector< plugin_ptr >& vec )
{
    return d_->select_clsids( regex, vec );
}

//////////////////

using namespace adplugin::internal;

void
manager_data::visit( adplugin::plugin * plugin, const char * adpluginspec )
{
    if ( adpluginspec == 0 || plugin == 0 )
        return;

    // make it unique
	auto it = std::find_if( plugins_.begin(), plugins_.end(), [&](const map_type::value_type& d){
		return d.second == (*plugin);
	});
	if ( it == plugins_.end() )
		additionals_.push_back( plugin_data( plugin ) );
}

void
manager_data::populated()
{
#if defined _DEBUG || defined DEBUG
    adportable::debug(__FILE__, __LINE__) << "==> populated";
	std::for_each( plugins_.begin(), plugins_.end(), [&](const map_type::value_type& d){
            adportable::debug(__FILE__, __LINE__) << "\t" << d.second.iid();
        });
    adportable::debug(__FILE__, __LINE__) << "<== populated";
#endif
}

bool
manager_data::install( QLibrary& lib, const std::string& adpluginspec, const std::string& specxml )
{
    if ( plugins_.find( adpluginspec ) != plugins_.end() )
        return true; // already in, so that does not need unload() call
    typedef adplugin::plugin * (*factory)();

    if ( lib.isLoaded() ) {
        factory f = reinterpret_cast< factory >( lib.resolve( "adplugin_plugin_instance" ) );
        if ( f ) {
            adplugin::plugin * pptr = f();
            adplugin::plugin_ptr ptr( pptr, false );
            if ( ptr ) {
                ptr->clsid_ = adpluginspec;
                ptr->spec_  = specxml;
				plugins_[ adpluginspec ] = plugin_data( ptr );

                ptr->accept( *this, adpluginspec.c_str() );

                return true;
            }
        }
    }
    return false;
}

plugin_ptr
manager_data::select_iid( const char * regex )
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

plugin_ptr
manager_data::select_clsid( const char * regex )
{
    return 0;
}

size_t
manager_data::select_iids( const char * regex, std::vector< plugin_ptr >& vec )
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
manager_data::select_clsids( const char * regex, std::vector< plugin_ptr >& vec )
{
    return 0;
}
