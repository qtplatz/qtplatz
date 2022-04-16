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

#include "make_filename.hpp"
#include <adportable/debug.hpp>
#include <adportfolio/folium.hpp>
#include <adportfolio/portfolioimpl.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

namespace {

    boost::filesystem::path make_filename_string( const portfolio::Folium& folium ) {
        auto name = folium.name();
        std::replace( name.begin(), name.end(), '/', '_' );
        boost::algorithm::trim( name ); // remove leading and trailing spaces
        return name + L".svg"; // add temporary extension for avoiding wrong extension substitution on replace_extension call
    }

    boost::filesystem::path make_directory_string( const QString& lastDir ) {
        auto dir  = boost::filesystem::path( lastDir.toStdString() );
        while ( !dir.empty() &&
                !( boost::filesystem::exists( dir ) && boost::filesystem::is_directory( dir ) ) ) {
            dir = dir.parent_path();
        }
        return dir;
    }

    boost::filesystem::path __make_filename( const portfolio::Folium& folium
                                             , std::string&& insertor
                                             , const QString& lastDir, const char * extension ) {

        auto stem = boost::filesystem::path( folium.impl()->fullpath() ).stem().string()
            + ( insertor.empty() ? "_" : insertor )
            + make_filename_string( folium ).string();
        auto dir  = make_directory_string( lastDir );
        if ( dir.empty() ) {
            dir = make_directory_string( QString::fromStdWString( folium.impl()->fullpath() ) );
        }
        return ( dir / stem ).replace_extension( extension );
    }

}



namespace dataproc {

    // enum PrintFormatType { PDF, SVG };
    template<>
    QString make_filename< SVG >::operator()( const portfolio::Folium& folium, std::string&& insertor, const QString& lastDir )
    {
        return QString::fromStdString( __make_filename( folium, std::move( insertor ), lastDir, ".svg" ).string() );
    }

    template<>
    QString make_filename< PDF >::operator()( const portfolio::Folium& folium, std::string&& insertor, const QString& lastDir )
    {
        return QString::fromStdString( __make_filename( folium, std::move( insertor ), lastDir, ".pdf" ).string() );
    }

}
