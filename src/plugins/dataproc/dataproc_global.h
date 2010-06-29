#ifndef ANALYSIS_GLOBAL_H
#define ANALYSIS_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(ANALYSIS_LIBRARY)
#  define DATAPROCSHARED_EXPORT Q_DECL_EXPORT
#else
#  define DATAPROCSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // ANALYSIS_GLOBAL_H
