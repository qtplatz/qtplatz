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

#ifndef FONT_HPP
#define FONT_HPP

#pragma once

class QFont;
class QString;

namespace qtwrapper {

    enum fontFamily {
        fontDefault
        , fontTableHeader
        , fontTableBody
        , fontTableText
        , fontTableNumber
        , fontAxisLabel
        , fontAxisTitle
        , fontPlotTitle
        , fontPlotFooter
        , fontForm
        , nbrFontFamily
    };

    enum fontSize {
        fontSizeTiny
        , fontSizeFootnote
        , fontSizeSmall
        , fontSizePlotTitle
        , fontSizePlotFooter
        , fontSizeNormal
        , fontSizeLarge
        , fontSizeHuge
        , nbrFontSize
    };

    class font {
    public:
        font();
        // static QFont& setFont( QFont& font, fontSize size, fontFamily family );
        // static QFont& setSize( QFont& font, fontSize size );
        // static QFont& setFamily( QFont& font, fontFamily family );
        QFont& operator () ( QFont&& font, fontSize size, fontFamily family );
        QFont& operator () ( QFont&& font, double scale );
    };

    struct font_size {
        template<typename T=QString> T operator()( int sz ) const;
    };
}

#endif // FONT_HPP
