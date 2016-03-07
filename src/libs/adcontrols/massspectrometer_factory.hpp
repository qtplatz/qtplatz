// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
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
#include <boost/uuid/uuid.hpp>
#include <string>
#include <memory>

namespace boost { namespace uuids { struct uuid; } }

namespace adcontrols {

    class MassSpectrometer;
    class datafile;
    
    class ADCONTROLSSHARED_EXPORT massspectrometer_factory : public std::enable_shared_from_this< massspectrometer_factory > {
    protected:
        massspectrometer_factory(void);
    public:
        virtual ~massspectrometer_factory(void);

        virtual const wchar_t * name() const = 0;
        virtual const boost::uuids::uuid& objclsid() const = 0;  // object clsid (uuid) that will be created by this factory
        virtual const char * objtext() const = 0;                // object text (human readable id) that will be created by this factory

        [[deprecated]] virtual MassSpectrometer * get( const wchar_t * modelname ) = 0; // depricated
        virtual std::shared_ptr< MassSpectrometer > create( const wchar_t * modelname, adcontrols::datafile * ) const = 0;
        virtual bool is_canonical_name( const wchar_t * )  const { return false; }   
    };

    namespace helper
    {
        template <std::size_t... Ts>
        struct index {};
        
        template <std::size_t N, std::size_t... Ts>
        struct gen_seq : gen_seq<N - 1, N - 1, Ts...> {};
        
        template <std::size_t... Ts>
        struct gen_seq<0, Ts...> : index<Ts...> {};
    }
    
    template< typename massspectrometer_type, typename... Args >
    class massspectrometer_factory_type : public massspectrometer_factory {

        massspectrometer_factory_type( const massspectrometer_factory_type& ) = delete;
        massspectrometer_factory_type& operator = ( const massspectrometer_factory_type& ) = delete;

        boost::uuids::uuid objclsid_;
        std::string objtext_;
        const std::tuple<Args...> args_;

    public:
        massspectrometer_factory_type( const std::string& objtext
                                       , const boost::uuids::uuid& objclsid
                                       , Args&&... args ) : objclsid_( objclsid )
                                                          , objtext_( objtext )
                                                          , args_( std::make_tuple( std::forward<Args>( args )... ) ) {
        }

        ~massspectrometer_factory_type() {
        }

        template< std::size_t... Is>
        std::shared_ptr< MassSpectrometer > __creator( const std::tuple<Args...>& args, helper::index<Is...> ) const {
            return std::make_shared< massspectrometer_type >( std::get<Is>(args)... );
        }

        std::shared_ptr< MassSpectrometer > creator( const std::tuple<Args...>& args ) const {
            return __creator( args_, helper::gen_seq<sizeof...(Args)>{} );
        }

        //--------
        const wchar_t * name() const override { return L""; }

        [[deprecated]] MassSpectrometer * get( const wchar_t * modelname ) override { return 0; } // depricated 

        std::shared_ptr< MassSpectrometer > create( const wchar_t * /* modelname */, adcontrols::datafile * ) const override {
            return creator( args_ );
        }

        bool is_canonical_name( const wchar_t * )  const override { return false; }   

        const char * objtext() const override { return objtext_.c_str(); }

        const boost::uuids::uuid& objclsid() const override { return objclsid_; }
    };

    
}

