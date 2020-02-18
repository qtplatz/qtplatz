/**************************************************************************
** Copyright (C) 2020 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include <adportable/optional.hpp>
#include <vector>

class QUrl;
class QClipboard;
class QString;
class QByteArray;

namespace adwidgets {

    class MolTableHelper {
    public:
        struct SmilesToSVG {
            adportable::optional< std::tuple< QString, QByteArray > > // formula, svg
            operator()( const QString& smiles ) const;
        };

        struct SDMolSupplier {
            typedef std::tuple< QString, QString, QByteArray > value_type; // formula,smiles,svg

            std::vector< value_type > operator()( const QUrl& ) const;           // drag&drop
            std::vector< value_type > operator()( const QClipboard* ) const;     // paste
        };
    };

}
