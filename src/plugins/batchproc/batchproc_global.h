#ifndef BATCHPROC_GLOBAL_H
#define BATCHPROC_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(BATCHPROC_LIBRARY)
#  define BATCHPROCSHARED_EXPORT Q_DECL_EXPORT
#else
#  define BATCHPROCSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // BATCHPROC_GLOBAL_H
