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

#include <string>
#include <memory>

namespace adcontrols { class MassSpectrum; }
namespace portfolio { class Folium; }

namespace dataproc {

    struct datafolder {
        int idx;
        std::wstring display_name; // fileneme::folium.name
        std::wstring idFolium;
        std::wstring idCentroid;
        std::weak_ptr< adcontrols::MassSpectrum > profile;    // usually profile, TBD for histogram data
        std::weak_ptr< adcontrols::MassSpectrum > centroid;  // centroid

        datafolder( int _0 = 0
                    , const std::wstring& _1 = std::wstring()
                    , const std::wstring& _2 = std::wstring()
                    , const std::wstring& _3 = std::wstring() );
        datafolder( int _idx
                    , const std::wstring& _display_name
                    , portfolio::Folium& folium );

        datafolder( const datafolder& t );

    };
}

