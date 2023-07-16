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

#include "portfolio_global.h"
#include <string>
#include <vector>
#include <memory>

namespace boost { namespace uuids { struct uuid; } }

namespace portfolio {

    namespace internal {
        class PortfolioImpl;
    }

    class Folium;
    class Folder;

    // typedef std::pair< std::wstring, std::wstring > attribute_type;
    // typedef std::vector< attribute_type > attributes_type;

    class PORTFOLIOSHARED_EXPORT Portfolio {
    public:
        ~Portfolio();
        Portfolio();
        Portfolio( const Portfolio& );
        Portfolio( const std::wstring& xml );
		Portfolio( const std::string& xml );
        Portfolio( std::shared_ptr< internal::PortfolioImpl > );

        std::vector<Folder> folders();
        const std::vector<Folder> folders() const;
        Folium findFolium( const std::wstring& id ) const;
        Folium findFolium( const boost::uuids::uuid& id ) const;

        Folder findFolder( const std::wstring& name ) const;

        // create new from scratch
        bool create_with_fullpath( const std::wstring& fullpath );
        Folder addFolder( const std::wstring& name, bool uniq = true );

        template< typename T > std::vector< std::pair< T, T > > attributes() const;

        std::string xml() const;
		std::wstring wxml() const;
		std::wstring fullpath() const;

        const std::vector< std::tuple< std::string, std::string > >& erased_dataIds() const;

        // for debugging convension
        bool save( const std::wstring& filename ) const;

        static boost::uuids::uuid uuidFromString( const std::string& );
        static boost::uuids::uuid uuidFromString( const std::wstring& );

    private:
        std::shared_ptr< internal::PortfolioImpl > impl_;
    };

    template<> std::vector< std::pair< std::wstring, std::wstring > > Portfolio::attributes< std::wstring >() const;
    template<> std::vector< std::pair< std::string, std::string > > Portfolio::attributes< std::string >() const;

}
