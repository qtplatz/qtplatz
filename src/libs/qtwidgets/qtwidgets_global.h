#ifndef QTWIDGETS_GLOBAL_H
#define QTWIDGETS_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(QTWIDGETS_LIBRARY)
#  define QTWIDGETSSHARED_EXPORT Q_DECL_EXPORT
#else
#  define QTWIDGETSSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // QTWIDGETS_GLOBAL_H
