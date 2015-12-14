// This is a -*- C++ -*- header.
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

#pragma once

#include "adplugin_global.h"
#include <memory>
#include <string>
#include <typeinfo>
#include <compiler/pragma_warning.hpp>

namespace adplugin {

    namespace internal { class manager_data; }

    class visitor;
    class plugin;

#if defined _MSC_VER
    ADPLUGINSHARED_TEMPLATE_EXPORT template class ADPLUGINSHARED_EXPORT std::weak_ptr < plugin > ;
#endif

    ////////////////////////
    
    class ADPLUGINSHARED_EXPORT plugin : public std::enable_shared_from_this<plugin> {

        plugin( const plugin& ) = delete;
        plugin& operator = ( const plugin& ) = delete;

        pragma_msvc_warning_push_disable_4251
        std::string clsid_;  // full path to .adplugin
        std::string spec_;   // context of .adplugin
        pragma_msvc_warning_pop

        virtual void * query_interface_workaround( const char * /* typename */ ) { return 0; }

    public:
		virtual ~plugin();
        plugin();

        void setConfig( const std::string& adpluginspec, const std::string& xml );

        std::shared_ptr< plugin > pThis();
        std::shared_ptr< const plugin > pThis() const;

        virtual void accept( visitor&, const char * adplugin ) = 0;
        virtual const char * iid() const = 0;
        virtual const char * clsid() const { return clsid_.c_str(); }       // adplugin name
        virtual const char * adpluginspec() const { return spec_.c_str(); } // context of adplugin file (may be xml but may not be)
        
        template<typename T> T* query_interface() {
            // dynamic_cast across the shared object loaded by dlopen doesn't work even tried with
            // QLibrary.setLoadHints(QLibrary::ResolveAllSymbolsHint| QLibrary::ExportExternalSymbolsHint)
            // a.k.a. RTLD_NOW|RTLD_GLOBAL for dlopen
            // This hit to a problem on Apple clang 4.0 based on LLVM 3.1svn
            // 2013 Jun 1st, -toshi
            T* p( 0 );
            try { p = dynamic_cast<T*>( this ); } catch ( ... ) {}
            if ( !p )
                p = reinterpret_cast<T*>( query_interface_workaround( typeid( T ).name() ) );
            return p;
        }
    };

}


