/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
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

#include "ieditorfactory.hpp"
#include <functional>
#include <QString>
#include <boost/uuid/uuid.hpp>

namespace adextension {

#if 0
    template< typename Editor>
    class iEditorFactoryT : public iEditorFactory {

        QString title_;
        boost::uuids::uuid clsid_;
        iEditorFactory::METHOD_TYPE mtype_;

    public:
        
		iEditorFactoryT( const QString& title
                         , iEditorFactory::METHOD_TYPE type ) : title_( title )
                                                              , clsid_( { 0 } )
                                                              , mtype_( type ) {
        }

		iEditorFactoryT( const QString& title
                         , const boost::uuids::uuid& clsid
                         , iEditorFactory::METHOD_TYPE type ) : title_( title )
                                                              , clsid_( clsid )
                                                              , mtype_( type ) {
        }

		~iEditorFactoryT() {
        }

        QWidget * createEditor( QWidget * parent = 0 ) const override {
            return new Editor( parent );
        }

        QString title() const override {
            return title_;
        }
        
        iEditorFactory::METHOD_TYPE method_type() const override {
            return mtype_;
        }

        const boost::uuids::uuid& clsid() const override {
            return clsid_;
        }
        
    };
#endif

    namespace helper
    {
        template <std::size_t... Ts>
        struct index {};
    
        template <std::size_t N, std::size_t... Ts>
        struct gen_seq : gen_seq<N - 1, N - 1, Ts...> {};
    
        template <std::size_t... Ts>
        struct gen_seq<0, Ts...> : index<Ts...> {};
    }
    
    template< typename Editor, typename... Args >
    class iEditorFactoryT : public iEditorFactory {

        iEditorFactoryT( const iEditorFactoryT& ) = delete;
        iEditorFactoryT& operator = ( const iEditorFactoryT& ) = delete;
        QString title_;
        boost::uuids::uuid clsid_;
        iEditorFactory::METHOD_TYPE mtype_;
        const std::tuple<Args...> args_;
    public:
        
		iEditorFactoryT( const QString& title
                         , iEditorFactory::METHOD_TYPE type
                         , Args&&... args ) : title_( title )
                                            , clsid_( {0})
                                            , mtype_( type )
                                            , args_( std::make_tuple(std::forward<Args>(args)...) ) {
        }

		iEditorFactoryT( const QString& title
                         , const boost::uuids::uuid& clsid
                         , iEditorFactory::METHOD_TYPE type
                         , Args&&... args ) : title_( title )
                                            , clsid_( clsid )
                                            , mtype_( type )
                                            , args_( std::make_tuple(std::forward<Args>(args)...) ) {
        }

		~iEditorFactoryT() {
        }

        template</*typename... Args,*/ std::size_t... Is>
        QWidget * __creator( QWidget * parent, const std::tuple<Args...>& args, helper::index<Is...> ) const {
            return new Editor( std::get<Is>(args)..., parent);
        }

        //template<> typename... Args>
        QWidget * creator( QWidget * parent, const std::tuple<Args...>& args ) const {
            return __creator( parent, args, helper::gen_seq<sizeof...(Args)>{} );
        }

        QWidget * createEditor( QWidget * parent = 0 ) const override {
            return creator( parent, args_ );
        }

        QString title() const override {
            return title_;
        }
        
        iEditorFactory::METHOD_TYPE method_type() const override {
            return mtype_;
        }

        const boost::uuids::uuid& clsid() const override {
            return clsid_;
        }
        
    };
    
}


