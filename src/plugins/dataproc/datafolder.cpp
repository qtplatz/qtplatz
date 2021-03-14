/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC
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

#include "datafolder.hpp"
#include "constants.hpp"
#include "dataprocessor.hpp"
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adportable/debug.hpp>
#include <adportfolio/folium.hpp>
#include <adportfolio/portfolio.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

using namespace dataproc;

datafolder::datafolder() : idx_(0)
{
}

datafolder::datafolder( int idx
                        , const std::wstring& fullpath
                        , const portfolio::Folium& folium ) : idx_( idx )
                                                            , display_name_( make_display_name( fullpath, folium ) )
                                                            , idFolium_( folium.id() )
                                                            , idfolium_( folium.uuid() )
{
    ADDEBUG() << "display_name: " << display_name_.toStdString();

    if ( auto raw = portfolio::get< adcontrols::MassSpectrumPtr >( folium ) ) {
        profile_ = raw; // maybe profile or histogram
        if ( raw->isHistogram() ) {
            if ( auto fi = portfolio::find_first_of( folium.attachments()
                                                     , [](const auto& a){ return a.name() == Constants::F_PROFILED_HISTOGRAM; }) ) {
                if ( auto ptr = portfolio::get< adcontrols::MassSpectrumPtr >( fi ) ) {
                    profiledHistogram_ = ptr;
                }
            }
        }
        if ( auto fi = portfolio::find_last_of( folium.attachments()
                                                , [](const auto& a){ return a.name() == Constants::F_CENTROID_SPECTRUM; }) ) {
            if ( auto ptr = portfolio::get< adcontrols::MassSpectrumPtr >( fi ) ) {
                centroid_ = ptr;
            }
        }
    } else if ( auto raw = portfolio::get< adcontrols::ChromatogramPtr >( folium ) ) {
        chromatogram_ = raw;
    }
}

datafolder::datafolder( const datafolder& t ) : idx_( t.idx_ )
                                              , display_name_( t.display_name_ )
                                              , idFolium_( t.idFolium_ )
                                              , idCentroid_( t.idCentroid_ )
                                              , profile_( t.profile_ )
                                              , profiledHistogram_( t.profiledHistogram_ )
                                              , centroid_( t.centroid_ )
                                              , chromatogram_( t.chromatogram_ )
                                              , overlaySpectrum_( t.overlaySpectrum_ )
{
}

//static
QString
datafolder::make_display_name( Dataprocessor * dp, const portfolio::Folium& folium )
{
    auto pfolio = dp->getPortfolio();
    return make_display_name( pfolio.fullpath(), folium );
}

QString
datafolder::make_display_name( const std::wstring& fullpath, const portfolio::Folium& folium )
{
    const char inserter = ';';

    boost::filesystem::path path( fullpath );
    auto rpath = boost::filesystem::relative( path, path / "../.." );
    std::wstring name = rpath.wstring() + wchar_t( inserter ) + boost::algorithm::trim_copy( folium.name() );
    return QString::fromStdWString( name );
}

datafolder::operator bool() const
{
    return ( profile_.lock() || chromatogram_.lock() );
}

std::pair< std::shared_ptr< const adcontrols::MassSpectrum >, bool /* isHistogram */>
datafolder::get_profile() const
{
    if ( auto hist = this->profiledHistogram_.lock() )
        return { hist, true };
    else if ( auto prof = this->profile_.lock() )
        return { prof, false };
    else
        return { nullptr, false };
}
