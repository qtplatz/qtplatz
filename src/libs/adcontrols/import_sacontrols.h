// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#ifndef IMPORT_SACONTROLS_H
#define IMPORT_SACONTROLS_H

#import "C:/MassCentre3.1/bin/SAControlsW.dll" named_guids
#include <atlbase.h>

namespace adil {
     namespace internal {
          struct variant_bool {
             static VARIANT_BOOL to_variant( bool value ) { return value ? VARIANT_TRUE : VARIANT_FALSE; }
             static bool to_native( BOOL value ) { return value == VARIANT_FALSE ? false : true; }
             static bool to_native( VARIANT_BOOL value ) { return value == VARIANT_FALSE ? false : true; }
          };
     }
}


#endif // IMPORT_SACONTROLS_H
