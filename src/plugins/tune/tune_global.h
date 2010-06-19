#ifndef TUNE_GLOBAL_H
#define TUNE_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(TUNE_LIBRARY)
#  define TUNESHARED_EXPORT Q_DECL_EXPORT
#else
#  define TUNESHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // TUNE_GLOBAL_H
