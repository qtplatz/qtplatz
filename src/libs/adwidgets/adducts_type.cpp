/**************************************************************************
** Copyright (C) 2022-2022 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "adducts_type.hpp"
#include <adcontrols/constants.hpp>

namespace adwidgets {

    adducts_type::adducts_type()
    {
    }

    adducts_type::adducts_type( const std::tuple< std::string, std::string >& t )
        : adducts( t )
    {
    }

    QString
    adducts_type::get( adcontrols::ion_polarity polarity ) const
    {
        return polarity == adcontrols::polarity_positive
            ? QString::fromStdString( std::get< adcontrols::polarity_positive >( adducts ) )
            : QString::fromStdString( std::get< adcontrols::polarity_negative >( adducts ) );
    }

    void
    adducts_type::set( const QString& adduct, adcontrols::ion_polarity polarity )
    {
        ( polarity == adcontrols::polarity_positive
          ? std::get< adcontrols::polarity_positive >( adducts )
          : std::get< adcontrols::polarity_negative >( adducts ) ) = adduct.toStdString();
    }

}
