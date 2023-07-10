// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2022 MS-Cheminformatics LLC
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

#include "portfolio_global.h"
#include "node.hpp"
#include <boost/any.hpp>
#include <boost/optional.hpp>
#include <algorithm>
#include <functional>
#include <memory>

namespace portfolio {

    namespace internal {
        class PortfolioImpl;
    }

    class Folder;

    template<class T> inline bool is_type( boost::any& a ) {
        // see issue on boost, https://svn.boost.org/trac/boost/ticket/754, if any issue on shared object boundary
        return a.type() == typeid( T );
    }

    template<class T> inline bool is_type( const boost::any& a ) {
        // see issue on boost, https://svn.boost.org/trac/boost/ticket/754, if any issue on shared object boundary
        return a.type() == typeid( T );
    }

    template< typename... Args > bool is_any_of( const boost::any& a ) {
        return ( ( is_type<Args>( a ) ) || ... );
    }

    template< typename... Args > bool is_any_shared_of( const boost::any& a ) {
        return ( is_any_of< std::shared_ptr< Args >... >( a ) );
    }

    //---
    template < typename... Args > struct get_shared_of {};

    template < typename last_t > struct get_shared_of< last_t > {
        auto operator()( const boost::any& a ) const {
            if ( a.type() == typeid( std::shared_ptr< last_t  > ) ) {
                return boost::any_cast< std::shared_ptr< last_t > >( a );
            }
            return std::shared_ptr< last_t >{};
        }
    };

    template< typename first_t, typename... Args > struct get_shared_of< first_t, Args ... > {
        std::shared_ptr< first_t > operator()( const boost::any& a ) const {
            if ( a.type() == typeid( std::shared_ptr< first_t  > ) ) {
                return boost::any_cast< std::shared_ptr< first_t > >( a );
            }
            return get_shared_of< Args ... >()( a );
        }
    };
    //---

    class PORTFOLIOSHARED_EXPORT Folium : public internal::Node {
    public:
        ~Folium();
        Folium();
        Folium( const Folium& );
        Folium( const pugi::xml_node&, internal::PortfolioImpl * impl );
        // Folium( pugi::xml_node, internal::PortfolioImpl * impl );
    public:

        bool nil() const;
        bool empty() const;
        void operator = ( const boost::any& );
        operator boost::any& ();
        operator const boost::any& () const;
        inline boost::any& data() { return operator boost::any& (); }
        inline const boost::any& data() const { return operator const boost::any& (); }

        Folium assign( const boost::any&, const wchar_t * dataClass );

        std::vector< Folium > attachments();
        const std::vector< Folium > attachments() const;

        Folium parentFolium() const;
        Folder parentFolder() const;
		std::string fullpath( bool fullyqualified = true ) const;

        typedef std::vector< Folium > vector_type;

        template<class T> static vector_type::iterator find( vector_type::iterator it, vector_type::iterator ite ) {
			return std::find_if( it, ite, [=]( vector_type::value_type& f ){
                                              return is_type<T>( static_cast< boost::any& >( f ) );
				} );
        }

        // check type before any_cast, which is safe
        template<class T> static bool get( T& t, Folium& folium ) {
            boost::any& data = folium;
            if ( is_type<T>( data ) ) {
                t = boost::any_cast<T>(data);
                return true;
            }
            return false;
        }

        // check type before any_cast, which is safe
        template< typename T > boost::optional<T> get() {
            if ( is_type<T>( this->data() ) )
                return boost::any_cast<T>( this->data() );
            return boost::none;
        }

        // check type before any_cast, which is safe
        template< typename T > boost::optional<T> get() const {
            if ( is_type<T>( this->data() ) )
                return boost::any_cast<T>( this->data() );
            return boost::none;
        }

        // --- create/modify
        Folium addAttachment( const std::wstring& name );
		// bool removeAttachment( const std::wstring& name, bool removeContents = true );
        bool erase_attachment( const std::wstring& name, std::function< void( std::tuple< std::wstring, std::wstring > ) > callback );
        bool erase_attachments( std::function< void( std::tuple< std::wstring, std::wstring > ) > callback );
    };

    typedef std::vector< Folium > Folio;

    template<class T> T get( Folium& folium ) {
        if ( is_type<T>( folium.data() ) )
			return boost::any_cast<T>( folium.data() ); // may raise a boost::bad_any_cast exception
        return {};
    }

    template<class T> T get( const Folium& folium ) {
        if ( is_type<T>( folium.data() ) )
            return boost::any_cast<T>( folium.data() ); // may raise a boost::bad_any_cast exception
        return {};
    }

    template<class Pred> Folium find_first_of( Folio folio, Pred pred ) {
        auto it = std::find_if( folio.begin(), folio.end(), pred );
        if ( it != folio.end() )
            return *it;
        return Folium();
    }

    template<class Pred> Folium find_last_of( Folio folio, Pred pred ) {
        auto it = std::find_if( folio.rbegin(), folio.rend(), pred );
        if ( it != folio.rend() )
            return *it;
        return Folium();
    }

}
