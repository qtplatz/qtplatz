/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include <adplugin/plugin.hpp>
#include <memory>
#include <mutex>

namespace acqrsinterpreter {

    namespace helper
    {
        template <std::size_t... Ts>
        struct index {};
    
        template <std::size_t N, std::size_t... Ts>
        struct gen_seq : gen_seq<N - 1, N - 1, Ts...> {};
    
        template <std::size_t... Ts>
        struct gen_seq<0, Ts...> : index<Ts...> {};
    }
    
    template< typename T, typename _IID >
    class factory_plugin : public adplugin::plugin {
        factory_plugin() {
        }

    public:
        ~factory_plugin() {
        }

        static std::shared_ptr< factory_plugin > make_this() {
            struct make_shared_enabler : public factory_plugin<T,_IID> {};
            return std::make_shared< make_shared_enabler >();
        }
        
        // adplugin::plugin
        void accept( adplugin::visitor& visitor, const char * adplugin ) override {
            visitor.visit( this, adplugin );
        }

        const char * iid() const override {
            return _IID::value.c_str();
        }

        void * query_interface_workaround( const char * typname ) override {
            return 0;
        }
    };

}


