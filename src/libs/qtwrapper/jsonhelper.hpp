// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2020 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2020 MS-Cheminformatics LLC
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

#include <QJsonObject>
#include <QVariant>
#include <optional>
#include <adportable/optional.hpp>

namespace qtwrapper {
    struct JsonHelper {
#if __cplusplus >= 201703
        template< typename T > static std::optional<T> value( const QJsonObject& obj, const QString& key ) {
            auto it = obj.find( key );
            if ( it != obj.end() )
                return qvariant_cast<T>(*it);
            return std::nullopt;
        }
#else
        template< typename T > static boost::optional<T> value( const QJsonObject& obj, const QString& key ) {
            auto it = obj.find( key );
            if ( it != obj.end() )
                return qvariant_cast<T>(*it);
            return boost::none;
        }
#endif
    };
}
