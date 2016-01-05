// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
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

#include "adcontrols_global.h"
#include <string>
#include <memory>

namespace adcontrols {

    class MassSpectrometer;
    class datafile;
    
    class ADCONTROLSSHARED_EXPORT DataInterpreterFactory : public std::enable_shared_from_this< DataInterpreterFactory > {
    protected:
        DataInterpreterFactory( void ) {}
    public:
        virtual ~DataInterpreterFactory(void) {}
        virtual std::shared_ptr< DataInterpreter > create() const = 0;
        virtual bool is_canonical_name( const wchar_t * )  const { return false; }   
    };

    namespace datainterpreter {

        namespace helper
        {
            template <std::size_t... Ts>
            struct index {};
        
            template <std::size_t N, std::size_t... Ts>
            struct gen_seq : gen_seq<N - 1, N - 1, Ts...> {};
        
            template <std::size_t... Ts>
            struct gen_seq<0, Ts...> : index<Ts...> {};
        }

        //----------------
        
        template< typename datainterpreter_type, typename... Args >
        class factory_type : public DataInterpreterFactory {

            factory_type( const factory_type& ) = delete;
            factory_type& operator = ( const factory_type& ) = delete;

            const std::tuple<Args...> args_;

        public:
            factory_type( Args&&... args ) : args_( std::make_tuple( std::forward<Args>( args )... ) ) {
            }
            
            ~factory_type() {
            }

            template< std::size_t... Is>
            std::shared_ptr< DataInterpreter > __creator( const std::tuple<Args...>& args, helper::index<Is...> ) const {
                return std::make_shared< datainterpreter_type >( std::get<Is>(args)... );
            }

            std::shared_ptr< DataInterpreter > creator( const std::tuple<Args...>& args ) const {
                return __creator( args_, helper::gen_seq<sizeof...(Args)>{} );
            }

            std::shared_ptr< DataInterpreter > create() const override {
                return creator( args_ );
            }

            bool is_canonical_name( const wchar_t * )  const override { return false; }   
        };

    }
    
}

