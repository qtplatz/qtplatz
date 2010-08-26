// This is a -*- C++ -*- header.
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

