// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@scienceliaison.com
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

#include <QColor>

namespace adplot {
    namespace constants {

        namespace chromatogram {
            constexpr static QColor color_table [] = {
                QColor( 0x00, 0x00, 0xff )    // 0  blue
                , QColor( 0xff, 0x00, 0x00 )  // 1  red
                , QColor( 0x00, 0x80, 0x00 )  // 2  green
                , QColor( 0x4b, 0x00, 0x82 )  // 3  indigo
                , QColor( 0xff, 0x14, 0x93 )  // 4  deep pink
                , QColor( 0x94, 0x00, 0xd3 )  // 5  dark violet
                , QColor( 0x80, 0x00, 0x80 )  // 6  purple
                , QColor( 0xdc, 0x13, 0x4c )  // 7  crimson
                , QColor( 0x69, 0x69, 0x69 )  // 8  dim gray
                , QColor( 0x80, 0x80, 0x80 )  // 9  gray
                , QColor( 0xa9, 0xa9, 0xa9 )  //10  dark gray
                , QColor( 0xc0, 0xc0, 0xc0 )  //11  silver
                , QColor( 0xd3, 0xd3, 0xd3 )  //12  light gray
                , QColor( 0xd2, 0x69, 0x1e )  //13  chocolate
                , QColor( 0x00, 0x00, 0x8b )  //14  dark blue
                , QColor( 0xff, 0xff, 0xff )  //15  white
                , QColor( 0xff, 0x8c, 0x00 )  //16  dark orange
                , QColor( 0x00, 0x00, 0x00, 0x00 )  //17
            };

        }

    }
}
