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
#include <adcontrols/massspectrum.hpp>
#include <adportfolio/folium.hpp>
#include <adportfolio/portfolio.hpp>

using namespace dataproc;

datafolder::datafolder( int _0
                        , const std::wstring& _1
                        , const std::wstring& _2
                        , const std::wstring& _3 ) : idx( _0 )
                                                   , display_name( _1 )
                                                   , idFolium( _2 )
                                                   , idCentroid( _3 )
{
}

datafolder::datafolder( int _idx
                        , const std::wstring& _display_name
                        , portfolio::Folium& folium ) : idx( _idx )
                                                      , display_name( _display_name )
                                                      , idFolium( folium.id() )
{
    
    if ( auto ms = portfolio::get< adcontrols::MassSpectrumPtr >( folium ) ) {
        profile = ms; // maybe profile or histogram
    }
    
    portfolio::Folio atts = folium.attachments();
    auto itCentroid = std::find_if( atts.begin(), atts.end(), [] ( const portfolio::Folium& f ){
            return f.name() == Constants::F_CENTROID_SPECTRUM;
        } );

    if ( itCentroid != atts.end() ) {
        
        idCentroid = itCentroid->id();
        centroid = portfolio::get< adcontrols::MassSpectrumPtr >( *itCentroid );
        
    }
}

datafolder::datafolder( const datafolder& t ) : idx( t.idx )
                                              , idFolium( t.idFolium )
                                              , idCentroid( t.idCentroid )
                                              , display_name( t.display_name )
                                              , profile( t.profile )
                                              , centroid( t.centroid )
{
}

