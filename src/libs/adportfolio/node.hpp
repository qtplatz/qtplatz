// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include "portfolio_global.h"
#include <pugixml.hpp>
#include <string>
#include <vector>
#include <boost/uuid/uuid.hpp>

namespace portfolio {

    namespace internal {

        class PortfolioImpl;

        class PORTFOLIOSHARED_EXPORT Node {
        public:
            Node();
            Node( const Node& );
        protected:
            Node( const pugi::xml_node&, PortfolioImpl* impl );

        public:
            operator bool () const;

            // std::wstring name() const;
            template< typename char_type = wchar_t > std::basic_string< char_type > name() const;
            void name( const std::string& name );
            void name( const std::wstring& name );

            const boost::uuids::uuid& uuid() const;

            // id() returns identical string with uuid
            template< typename char_type = wchar_t > std::basic_string< char_type > id() const;
            void id( const std::wstring& );
            void id( const std::string& );

            bool isFolder() const;
            void isFolder( bool );

            template< typename char_type = wchar_t > std::basic_string< char_type > dataClass() const;
            void dataClass( const std::string& );
            void dataClass( const std::wstring& );

            std::wstring attribute( const std::wstring& ) const;
            std::string attribute( const std::string& ) const;
            void setAttribute( const std::wstring& key, const std::wstring& value );
            void setAttribute( const std::string& key, const std::string& value );

            // std::vector< std::pair<std::wstring, std::wstring> > attributes() const;
            template< typename T = std::string > std::vector< std::pair<T, T> > attributes() const;
            template< typename T > void appendAttributes( const std::vector< std::pair<T, T> >&, bool dontOverride = true );

            std::string attributes_json() const;

            static boost::uuids::uuid uuidFromString( const std::string& );

            const PortfolioImpl * impl() const { return impl_; };
            pugi::xpath_node select_node( const std::string& query );
            // std::wstring portfolio_fullpath() const;
            template< typename char_type > std::basic_string< char_type > filename() const;

        protected:
            pugi::xpath_node_set selectNodes( const std::wstring& query );
            pugi::xpath_node selectSingleNode( const std::wstring& query );

            pugi::xpath_node_set selectNodes( const std::string& query );
            pugi::xpath_node selectSingleNode( const std::string& query );

            pugi::xml_node addFolder( const std::wstring& name, PortfolioImpl* );
            pugi::xml_node addFolder( const std::string& name, PortfolioImpl* );
            pugi::xml_node addFolium( const std::wstring& name );
            pugi::xml_node addFolium( const std::string& name );
            pugi::xml_node addAttachment( const std::wstring& name, bool bUniq = true );
            pugi::xml_node addAttachment( const std::string& name, bool bUniq = true );

            std::vector< std::string >
            erase( const std::string& node, std::tuple< std::wstring, std::wstring > );

        protected:
            pugi::xml_node node_;
            PortfolioImpl* impl_;
            boost::uuids::uuid uuid_;
        };

        template<> PORTFOLIOSHARED_EXPORT std::string  Node::id() const;
        template<> PORTFOLIOSHARED_EXPORT std::wstring Node::id() const;
        template<> PORTFOLIOSHARED_EXPORT std::string Node::filename() const;
        template<> PORTFOLIOSHARED_EXPORT std::wstring Node::filename() const;
        template<> PORTFOLIOSHARED_EXPORT std::string Node::name() const;
        template<> PORTFOLIOSHARED_EXPORT std::wstring Node::name() const;
        template<> PORTFOLIOSHARED_EXPORT std::vector< std::pair< std::wstring, std::wstring > > Node::attributes< std::wstring >() const;
        template<> PORTFOLIOSHARED_EXPORT std::vector< std::pair< std::string, std::string > >   Node::attributes< std::string >() const;

        template<> PORTFOLIOSHARED_EXPORT std::string Node::dataClass() const;
        template<> PORTFOLIOSHARED_EXPORT std::wstring Node::dataClass() const;

        template<> PORTFOLIOSHARED_EXPORT void Node::appendAttributes( const std::vector< std::pair<std::wstring, std::wstring> >&, bool );
        template<> PORTFOLIOSHARED_EXPORT void Node::appendAttributes( const std::vector< std::pair<std::string, std::string> >&, bool );
    }
}
