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

#ifndef IMPORT_SAGRAPHICS_H
#define IMPORT_SAGRAPHICS_H

//#import "SAGraphicsU.dll" named_guids
#import "../../../../SATools/bin/SAGraphicsU.dll" named_guids
#include <atlbase.h>

namespace adwidgets {
	namespace ui {
		namespace internal {
			struct variant_bool {
				static VARIANT_BOOL to_variant( bool value ) { return value ? VARIANT_TRUE : VARIANT_FALSE; }
				static bool to_native( BOOL value ) { return value == VARIANT_FALSE ? false : true; }
				static bool to_native( VARIANT_BOOL value ) { return value == VARIANT_FALSE ? false : true; }
			};
		}
	}
}


#endif // IMPORT_SAGRAPHICS_H
