
#pragma once

#include <QtGlobal>

#if defined(AQMD3_LIBRARY)
#  define AQMD3SHARED_EXPORT Q_DECL_EXPORT
#else
#  define AQMD3SHARED_EXPORT Q_DECL_IMPORT
#endif
