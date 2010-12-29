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
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <string>

namespace SAGRAPHICSLib {
	struct ISADPTitle;
}

namespace adwidgets {
	namespace ui {

        class Font;

		class Title {
		public:
			~Title();
			Title( SAGRAPHICSLib::ISADPTitle * pi = 0 );
			void operator = ( const Title& );

			std::wstring text() const;
			void text( const std::wstring& );
			bool visible() const;
            void visible( bool );
			unsigned long color() const;
            void color( unsigned long );
            Font font();
		private:
			SAGRAPHICSLib::ISADPTitle * pi_;
		};
	}
}

