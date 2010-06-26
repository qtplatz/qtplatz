#ifndef ADBROKER_GLOBAL_H
#define ADBROKER_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(ADBROKER_LIBRARY)
#  define ADBROKER_EXPORT Q_DECL_EXPORT
#else
#  define ADBROKER_EXPORT Q_DECL_IMPORT
#endif

#endif // ADBROKER_GLOBAL_H
