#pragma once

#include <QtGlobal>

#if defined(ACQUIRE_LIBRARY)
#  define ACQUIRESHARED_EXPORT Q_DECL_EXPORT
#else
#  define ACQUIRESHARED_EXPORT Q_DECL_IMPORT
#endif
