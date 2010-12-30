// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

struct IDispatch;
#include <string>

namespace adwidgets {
    namespace ui {

        class Font  {
        public:
            ~Font();
            Font( IDispatch * pi = 0 );
            Font( const Font& );

            bool bold() const;
            void bold( bool );

            short charset() const;
            // HFONT hFont() const;
            bool italic() const;
            void italic( bool );

            std::wstring name() const;
            void name( const std::wstring& );

            long long size() const;
            void size( long long );

            bool strikethrough() const;
            bool underline() const;
            short weight() const;

        private:
            IDispatch * pi_;
        };

    }
}


