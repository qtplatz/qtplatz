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

#include "dataprocessor.hpp"
#include "document.hpp"
#include "sessionmanager.hpp"
#include "utility.hpp"
#include <adplot/plot.hpp>
#include <adportfolio/folium.hpp>
#include <adportfolio/portfolio.hpp>
#include <QFileDialog>

namespace portfolio {
    class Folium;
}

namespace dataproc {

    namespace utility {

        template<> std::pair<bool, QString>
        save_image_as< SVG >::operator ()( adplot::plot* plot, const std::wstring& foliumId, std::string&& insertor ) const
        {
            if ( auto dp = SessionManager::instance()->getActiveDataprocessor() ) {
                auto folium = dp->getPortfolio().findFolium( foliumId );
                if ( folium ) {
                    QFileDialog dlg( nullptr, QObject::tr( "Save SVG file" ) );
                    dlg.setDirectory( make_filename<SVG>()( folium, std::move(insertor), document::instance()->recentFile( Constants::GRP_SVG_FILES ) ) );
                    dlg.setAcceptMode( QFileDialog::AcceptSave );
                    dlg.setFileMode( QFileDialog::AnyFile );
                    dlg.setNameFilters( QStringList{ "SVG(*.svg)"} );
                    if ( dlg.exec() ) {
                        auto files = dlg.selectedFiles();
                        if ( !files.isEmpty() ) {
                            auto name = files.at( 0 );
                            adplot::plot::copyImageToFile( plot, name, "svg" );
                            document::instance()->addToRecentFiles( name, Constants::GRP_SVG_FILES );
                            return { true, name };
                        }
                    }

                }
            }
            return { false, {} };
        }
        //<----------

        std::optional< std::filesystem::path >
        save_spectrum_as::operator ()( const portfolio::Folium& parent, const portfolio::Folium& folium, std::string&& insertor ) const
        {
            if ( folium ) {
                QFileDialog dlg( nullptr, QObject::tr( "Save Spectrum As" ) );
                dlg.setDirectory( make_filename<TXT>()( parent, std::move(insertor), document::instance()->recentFile( Constants::GRP_SAVEAS_FILES ) ) );
                dlg.setAcceptMode( QFileDialog::AcceptSave );
                dlg.setFileMode( QFileDialog::AnyFile );
                dlg.setNameFilters( QStringList{ "TXT (*.txt)", "QtPlatz (*.adfs)"} );
                if ( dlg.exec() ) {
                    auto files = dlg.selectedFiles();
                    if ( !files.isEmpty() ) {
                        auto name = files.at( 0 );
                        document::instance()->addToRecentFiles( name, Constants::GRP_SAVEAS_FILES );
                        return name.toStdString();
                    }
                }
            }
            return {};
        }
        //<----------

        std::optional< std::filesystem::path >
        save_chromatogram_as::operator ()( const portfolio::Folium& folium, std::string&& insertor ) const
        {
            if ( folium ) {
                QFileDialog dlg( nullptr, QObject::tr( "Save Chromatogram As" ) );
                dlg.setDirectory( make_filename<TXT>()( folium, std::move(insertor), document::instance()->recentFile( Constants::GRP_SAVEAS_FILES ) ) );
                dlg.setAcceptMode( QFileDialog::AcceptSave );
                dlg.setFileMode( QFileDialog::AnyFile );
                dlg.setNameFilters( QStringList{ "TXT (*.txt)", "QtPlatz (*.adfs)"} );
                if ( dlg.exec() ) {
                    auto files = dlg.selectedFiles();
                    if ( !files.isEmpty() ) {
                        auto name = files.at( 0 );
                        document::instance()->addToRecentFiles( name, Constants::GRP_SAVEAS_FILES );
                        return name.toStdString();
                    }
                }
            }
            return {};
        }
        //<----------




    } // namespace utility
}
