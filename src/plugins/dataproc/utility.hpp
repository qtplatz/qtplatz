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

#pragma once

#include <QString>
#include "make_filename.hpp"
#include <adportable/optional.hpp>
#include <boost/uuid/uuid.hpp>
#include <filesystem>

namespace portfolio {
    class Folium;
}

namespace adplot {
    class plot;
}

namespace dataproc {
    namespace utility {

        template< PrintFormatType >
        struct save_image_as {
            // std::pair<bool, QString> operator ()( adplot::plot*, const std::wstring& foliumId, std::string&& insertor = {} ) const;
            template<typename id_type = std::wstring> std::optional<QString> operator ()( adplot::plot*, const id_type& foliumId, std::string&& insertor = {} ) const;

            std::optional<QString> operator ()( adplot::plot* ) const;
        };

        template<>
        template<> std::optional<QString>
        save_image_as< SVG >::operator ()( adplot::plot*, const portfolio::Folium&, std::string&& ) const;

        template<>
        template<> std::optional<QString>
        save_image_as< SVG >::operator ()( adplot::plot*, const std::wstring&, std::string&& ) const;

        template<>
        template<> std::optional<QString>
        save_image_as< SVG >::operator ()( adplot::plot*, const boost::uuids::uuid&, std::string&& ) const;

        template<> std::optional<QString>
        save_image_as< SVG >::operator ()( adplot::plot* ) const;
    }

    namespace utility {

        enum MSDataOutType { Profile, Centrooid };


        struct save_spectrum_as {
            adportable::optional< std::filesystem::path > operator ()( const portfolio::Folium& name
                                                                         , const portfolio::Folium& target
                                                                         , std::string&& insertor = {} ) const;
        };

        struct save_chromatogram_as {
            adportable::optional< std::filesystem::path > operator ()( const portfolio::Folium&
                                                                         , std::string&& insertor = {} ) const;
        };

        struct export_mslock_as {
            adportable::optional< std::filesystem::path > operator ()( const portfolio::Folium& name
                                                                         , std::string&& insertor = {} ) const;
        };

    }
}
