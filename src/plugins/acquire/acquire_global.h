#ifndef ACQUIRE_GLOBAL_H
#define ACQUIRE_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(ACQUIRE_LIBRARY)
#  define ACQUIRESHARED_EXPORT Q_DECL_EXPORT
#else
#  define ACQUIRESHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // ACQUIRE_GLOBAL_H
