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

            std::wstring name() const;
            void name( const std::wstring& name );

            const boost::uuids::uuid& uuid() const;
            std::wstring id() const;
            void id( const std::wstring& );

            bool isFolder() const;
            void isFolder( bool );

            std::wstring dataClass() const;
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
            std::wstring portfolio_fullpath() const;

        protected:
            pugi::xpath_node_set selectNodes( const std::wstring& query );
            pugi::xpath_node selectSingleNode( const std::wstring& query );

            pugi::xpath_node_set selectNodes( const std::string& query );
            pugi::xpath_node selectSingleNode( const std::string& query );

            pugi::xml_node addFolder( const std::wstring& name, PortfolioImpl* );
            pugi::xml_node addFolium( const std::wstring& name );
            pugi::xml_node addAttachment( const std::wstring& name, bool bUniq = true );

            std::vector< std::string >
            erase( const std::string& node, std::tuple< std::wstring, std::wstring > );

        protected:
            pugi::xml_node node_;
            PortfolioImpl* impl_;
            boost::uuids::uuid uuid_;
        };

        template<> std::vector< std::pair< std::wstring, std::wstring > > Node::attributes< std::wstring >() const;
        template<> std::vector< std::pair< std::string, std::string > > Node::attributes< std::string >() const;

        template<> void Node::appendAttributes( const std::vector< std::pair<std::wstring, std::wstring> >&, bool );
        template<> void Node::appendAttributes( const std::vector< std::pair<std::string, std::string> >&, bool );
    }
}
