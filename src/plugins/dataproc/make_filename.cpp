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
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <filesystem>
#include <format>
#include <regex>

namespace {

    std::filesystem::path make_filename_string( const portfolio::Folium& folium ) {
        auto name = folium.name();
        std::replace( name.begin(), name.end(), '/', '_' );
        std::replace( name.begin(), name.end(), ' ', '_' );
        boost::algorithm::trim( name ); // remove leading and trailing spaces
        return name; // add temporary extension for avoiding wrong extension substitution on replace_extension call
    }

    std::filesystem::path make_directory_string( const QString& lastDir ) {
        auto dir  = std::filesystem::path( lastDir.toStdString() );
        while ( !dir.empty() &&
                !( std::filesystem::exists( dir ) && std::filesystem::is_directory( dir ) ) ) {
            dir = dir.parent_path();
        }
        return dir;
    }

    std::filesystem::path __make_filename( const portfolio::Folium& folium
                                             , std::string&& insertor
                                             , const QString& lastDir, const char * extension ) { // must contains '.'
        std::ostringstream o;
        auto stem = std::filesystem::path( folium.filename<char>() ).stem().string();
        if ( insertor.empty() )
            insertor = "_";

        // SFE10sSFC-JBA941_01_0800uL2,5%24,6MPa_He120mL_DC=0470FT-000_ARA_05,0uM_0,2uL_TL_E1-Skimmer1-1mm_93uA_0002_Seg1Ev1__3,_m_z_303.20_neg.svg
        // Shimadzu 8060 CDF filename pattern
        std::regex re(R"(_(\d+)_Seg\dEv\d)");
        std::smatch matches;
        std::string runno{};
        if ( std::regex_search(stem, matches, re)) {
            runno = matches[1];
        }

        o << std::filesystem::path( folium.filename<char>() ).parent_path().filename().string(); // parentDir 'YYYY-MM-DD'
        o << std::format( "{}{}{}{}", insertor, runno, insertor, stem );
        o << std::format( "{}{}{}", insertor, make_filename_string( folium ).string(), extension );      // replace '/' -> '_'

        auto dir  = make_directory_string( lastDir );
        if ( dir.empty() ) {
            dir = make_directory_string( QString::fromStdWString( folium.filename<wchar_t>() ) );
        }
        auto destname = dir / o.str();
        if ( std::filesystem::exists( destname ) ) {
            int n{1};
            auto name = destname.replace_extension(); // remove extension
            do {
                destname = std::format( "{}({}){}", name.string(), n++, extension );
                // destname = ( boost::format("%s(%d)%s") % name.string() % n++ % extension ).str();
            } while ( std::filesystem::exists( destname ) );
        }
        return destname;
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

    template<>
    QString make_filename< TXT >::operator()( const portfolio::Folium& folium, std::string&& insertor, const QString& lastDir )
    {
        return QString::fromStdString( __make_filename( folium, std::move( insertor ), lastDir, ".txt" ).string() );
    }

    template<>
    QString make_filename< JSON >::operator()( const portfolio::Folium& folium, std::string&& insertor, const QString& lastDir )
    {
        return QString::fromStdString( __make_filename( folium, std::move( insertor ), lastDir, ".json" ).string() );
    }

}
