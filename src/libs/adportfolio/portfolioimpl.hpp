// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include <boost/any.hpp>
#include <memory>
#include <set>
#include <map>
#include <pugixml.hpp>
#include "node.hpp"

namespace portfolio {

    class Folium;
    class Folder;
    class Portfolio;

    namespace internal {

        // Portfolio is a root folder

        class PortfolioImpl : public Node
                            , public std::enable_shared_from_this< PortfolioImpl > {
            PortfolioImpl(const PortfolioImpl &) = delete;
            PortfolioImpl();
        public:
            PortfolioImpl( const std::string& xml );
            operator bool () const { return isXMLLoaded_; }
            const std::wstring fullpath() const;
            std::vector<Folder> selectFolders( const std::wstring& );
            Folium selectFolium( const std::wstring& query );

            boost::any& find( const std::wstring& id );
            void assign( const std::wstring& id, const boost::any& );

            Folder findFolder( const std::wstring& );

            ///////////////  creation ///////////////
            bool create_with_fullpath( const std::wstring& );
            Folder addFolder( const std::wstring&, bool uniq );
            Folder addFolder( const std::string&, bool uniq );
            pugi::xml_document& getDocument() { return doc_; }
            static std::wstring newGuid();

            std::vector< std::tuple< std::string, std::string > > collect_garbage() const;
            bool erase_data( const std::string& name, const std::string& dataId );

        private:
            friend class portfolio::Portfolio;
            friend class Node;

            bool isXMLLoaded_;
            std::map< std::wstring, boost::any > db_;
        protected:
            pugi::xml_document doc_;
            std::vector<std::tuple<std::string, std::string>>  removed_list_; // name,dataId
            friend class Portfolio;
        };
    }
}
