#ifndef SEQUENCE_GLOBAL_H
#define SEQUENCE_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(SEQUENCE_LIBRARY)
#  define SEQUENCESHARED_EXPORT Q_DECL_EXPORT
#else
#  define SEQUENCESHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // SEQUENCE_GLOBAL_H
