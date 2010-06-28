#ifndef ADMACHINE_GLOBAL_H
#define ADMACHINE_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(ADMACHINE_LIBRARY)
#  define ADMACHINESHARED_EXPORT Q_DECL_EXPORT
#else
#  define ADMACHINESHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // ADMACHINE_GLOBAL_H
