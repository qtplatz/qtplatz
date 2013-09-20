// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

namespace portfolio {

    namespace internal {
        class PortfolioImpl;
    }

    class Folder;

    template<class T> inline bool is_type( boost::any& a ) {
        // see issue on boost, https://svn.boost.org/trac/boost/ticket/754
#if defined __GNUC__
        return std::string( a.type().name() ) == typeid( T ).name();
#else
        return a.type() == typeid( T );
#endif            
    }

    class PORTFOLIOSHARED_EXPORT Folium : public internal::Node {
    public:
        ~Folium();
        Folium();
        Folium( const Folium& );
        Folium( pugi::xml_node&, internal::PortfolioImpl * impl );
        Folium( pugi::xml_node, internal::PortfolioImpl * impl );
    public:

        bool empty() const;
        void operator = ( const boost::any& );
        operator boost::any& ();
        operator const boost::any& () const;
        inline boost::any& data() { return operator boost::any& (); }
        inline const boost::any& data() const { return operator const boost::any& (); }

        void assign( const boost::any&, const wchar_t * dataClass );

        std::vector< Folium > attachments();
        const std::vector< Folium > attachments() const;
        Folder getParentFolder();
		std::string fullpath( bool fullyqualified = true ) const;

        typedef std::vector< Folium > vector_type;

        template<class T> static vector_type::iterator find_first_of( vector_type::iterator it, vector_type::iterator ite ) {
			return std::find_if( it, ite, [=]( vector_type::value_type& f ){ 
                    return is_type<T>( static_cast< boost::any& >( f ) );
				} );
        }

        template<class T> static vector_type::iterator find_if( vector_type::iterator it, vector_type::iterator ite ) {
			return std::find_if( it, ite, [=]( vector_type::value_type& f ){ 
                    return is_type<T>( static_cast< boost::any& >( f ) );
				} );
        }

        template<class T> static bool get( T& t, Folium& folium ) {
            boost::any& data = folium;
            if ( is_type<T>( data ) ) {
                t = boost::any_cast<T>(data);
                return true;
            }
            return false;
        }

        // --- create/modify
        Folium addAttachment( const std::wstring& name );
		bool removeAttachment( const std::wstring& name, bool removeContents = true );
    };

    typedef std::vector< Folium > Folio;

}


