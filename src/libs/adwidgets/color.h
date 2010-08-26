// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

namespace SAGRAPHICSLib {
    struct ISADPColor;
}

namespace adwidgets {
	namespace ui {

		class Color {
		public:
			Color(void);
			~Color(void);
			void operator = ( const Color& );
			unsigned long /* OLE_COLOR */ value() const;
            void value( const /* OLE_COLOR */ unsigned long& );
			void rgb( int r, int g, int b );
			int red() const;
			int green() const;
			int blue() const;
		private:
			SAGRAPHICSLib::ISADPColor * pi_;
		};

	}
}
