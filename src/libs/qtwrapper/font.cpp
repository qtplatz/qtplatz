/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "font.hpp"
#include <QFont>

namespace qtwrapper {
#if defined Q_OS_MAC
    static int font_size_list [] = {
        6 // fontSizeTiny
        , 7 // fontSizeFootnote
        , 8 // fontSizeSmall
        , 9 // fontSizePlotTitle
        , 9 // fontSizePlotFooter
        , 10 // fontSizeNormal
        , 16 // fontSizeLarge
        , 24 // fontSizeHuge
    };
#else
    static int font_size_list [] = {
        6 // fontSizeTiny
        , 7 // fontSizeFootnote
        , 8 // fontSizeSmall
        , 9 // fontSizePlotTitle
        , 9 // fontSizePlotFooter
        , 10 // fontSizeNormal
        , 16 // fontSizeLarge
        , 24 // fontSizeHuge
    };
#endif

    static const char * font_family_list [] = {
        0 // fontDefault
        , "Verdena" // fontTableHeader
        , "Consolas" // fontTableBody
        , "Consolas" // fontTableText
        , "Consolas" // fontTableNumber
        , "Consolas" // fontAxisLabel
        , "Calibri" // fontAxisTitle
        , "Calibri" // fontPlotTitle
        , "Calibri" // fontPlotFooter
    };
}

using namespace qtwrapper;

font::font()
{
}

QFont&
font::setFamily( QFont& font, fontFamily family )
{
    if ( family >= 1 && family < sizeof( font_family_list )/sizeof( font_family_list[0] ) )
        font.setFamily( font_family_list[ family ] );

    return font;
}


QFont&
font::setSize( QFont& font, fontSize size )
{
    if ( size >= 0 && size < sizeof( font_size_list )/sizeof( font_size_list[0] ) )
        font.setPointSize( font_size_list[ size ] );

    return font;
}

QFont&
font::setFont( QFont& font, fontSize size, fontFamily family )
{
    return setFamily( setSize( font, size ), family );
}
