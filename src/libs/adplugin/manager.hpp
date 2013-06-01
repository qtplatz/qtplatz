// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2013 MS-Cheminformatics LLC
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
*
** Contact: toshi.hondo@scienceliaison.com
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

#pragma once

#include "adplugin_global.h"
#include "ifactory.hpp"
#include "orbLoader.hpp"
#include <string>
#include <vector>

class QString;
class QObject;
class QWidget;
class QLibrary;

namespace adportable {
    class Configuration;
    class Component;
}

namespace adplugin {

    namespace internal { class manager_data; }
	class plugin;
    
    class ADPLUGINSHARED_EXPORT manager {
    protected:
        manager();
        virtual ~manager();

		static manager * instance_;
		internal::manager_data * d_;
    public:
        static manager * instance();
        static void dispose();

        static std::string ior( const char * name ); // return broker::manager's ior
        static std::string iorBroker();

        // following method will be replaced with newly developped plugin loader

        virtual bool loadConfig( adportable::Configuration&, const std::wstring&, const wchar_t * query ) = 0;
	
        virtual adplugin::ifactory * loadFactory( const std::wstring& ) = 0;
        virtual bool unloadFactory( const std::wstring& ) = 0;
	
        virtual adplugin::orbLoader& orbLoader( const std::wstring& name ) = 0;
        virtual void register_ior( const std::string& name, const std::string& ior ) = 0;
        virtual const char * lookup_ior( const std::string& name ) = 0;
	
        static QWidget * widget_factory( const adportable::Configuration&, const wchar_t * path, QWidget * parent = 0 );

        // new plugin loader will be implemented below
		bool install( QLibrary&, const std::string& confname );

        plugin * select_iid( const char * iid );
        plugin * select_clsid( const char * clsid ); // return first match only
        size_t select_iids( const char * clsid, std::vector< plugin * >& );
        size_t select_clsids( const char * clsid, std::vector< plugin * >& );
        
    private:
	
    };
}
