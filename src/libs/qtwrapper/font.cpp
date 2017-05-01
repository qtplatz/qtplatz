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
#include <QString>
#include <QFont>
#include <array>

namespace qtwrapper {

#if defined Q_OS_MAC
# define FSIZE(x) ((x)*96/72)
#else
# define FSIZE(x) (x)
#endif

    static std::array<int, nbrFontSize> font_size_list = {
        { FSIZE(6) // fontSizeTiny
          , FSIZE(7) // fontSizeFootnote
          , FSIZE(8) // fontSizeSmall
          , FSIZE(9) // fontSizePlotTitle
          , FSIZE(9) // fontSizePlotFooter
          , FSIZE(10) // fontSizeNormal
          , FSIZE(16) // fontSizeLarge
          , FSIZE(24) } // fontSizeHuge
    };

    static std::array< const char *, nbrFontFamily > font_family_list = {
        { 0 // fontDefault
          , "Verdena" // fontTableHeader
          , "Consolas" // fontTableBody
          , "Consolas" // fontTableText
          , "Consolas" // fontTableNumber
          , "Consolas" // fontAxisLabel
          , "Calibri" // fontAxisTitle
          , "Calibri" // fontPlotTitle
          , "Calibri" // fontPlotFooter
          , "Calibri" // fontForm
        }
    };
}

using namespace qtwrapper;

font::font()
{
}

QFont&
font::operator () ( QFont&& font, fontSize size, fontFamily family )
{
    if ( family >= 1 && family < font_family_list.size() )
        font.setFamily( font_family_list[ family ] );

    if ( size >= 0 && size < font_size_list.size() )
        font.setPointSize( font_size_list[ size ] );
    return font;
}

QFont&
font::operator () ( QFont&& font, double scale )
{
    font.setPointSize( font.pointSize() * scale );
    return font;
}

namespace qtwrapper {

    template<> double
    font_size::operator()<double>( int sz ) const
    {
        return FSIZE( double(sz) );
    }

    template<> int
    font_size::operator()<int>( int sz ) const
    {
        return int( FSIZE( double(sz) ) + 0.5 );
    }

    template<> QString
    font_size::operator()<QString>( int sz ) const
    {
        return QString::number( FSIZE( double(sz) ) );
    }

}
