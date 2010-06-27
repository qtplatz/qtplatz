#pragma once

#include <QtCore/qglobal.h>

#if defined(ADCONTROLS_LIBRARY)
#  define ADCONTROLSSHARED_EXPORT Q_DECL_EXPORT
#else
#  define ADCONTROLSSHARED_EXPORT Q_DECL_IMPORT
#endif

