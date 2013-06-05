#ifndef ADORBMGR_GLOBAL_H
#define ADORBMGR_GLOBAL_H

#include <compiler/decl_export.h>
#include <compiler/decl_export.h>

#if defined(ADORBMGR_LIBRARY)
#  define ADORBMGRSHARED_EXPORT DECL_EXPORT
#else
#  define ADORBMGRSHARED_EXPORT DECL_IMPORT
#endif

#endif // ADORBMGR_GLOBAL_H
