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

#include <boost/any.hpp>
#include <map>
#include <xmlparser/pugixml.hpp>
#include "node.hpp"

namespace portfolio {

    class Folium;
    class Folder;

    namespace internal {

        // Portfolio is a root folder

        class PortfolioImpl : public Node {
            PortfolioImpl( const PortfolioImpl& );
        public:
            PortfolioImpl();
            PortfolioImpl( const std::wstring& xml );
            operator bool () const { return isXMLLoaded_; }
            const std::wstring fullpath() const;
            std::vector<Folder> selectFolders( const std::wstring& );
            Folium selectFolium( const std::wstring& query );

            boost::any& find( const std::wstring& id );
            void assign( const std::wstring& id, const boost::any& );

            ///////////////  creation ///////////////
            bool create_with_fullpath( const std::wstring& );
            Folder addFolder( const std::wstring&, bool uniq );
            pugi::xml_document& getDocument() { return doc_; }
            static std::wstring newGuid();
     
        private:
            bool isXMLLoaded_;
            std::map< std::wstring, boost::any > db_;
            pugi::xml_document doc_;
        };
    }
}


