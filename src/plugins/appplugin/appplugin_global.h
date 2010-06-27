#ifndef APPPLUGIN_GLOBAL_H
#define APPPLUGIN_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(APPPLUGIN_LIBRARY)
#  define APPPLUGIN_EXPORT Q_DECL_EXPORT
#else
#  define APPPLUGIN_EXPORT Q_DECL_IMPORT
#endif

#endif // APPPLUGIN_GLOBAL_H
