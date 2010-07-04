#ifndef IMPORT_SAGRAPHICS_H
#define IMPORT_SAGRAPHICS_H

//#import "SAGraphicsU.dll" named_guids
#import "../../../../SATools/bin/SAGraphicsU.dll" named_guids
#include <atlbase.h>

namespace adil {
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
