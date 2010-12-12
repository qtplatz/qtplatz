#ifndef PORTFOLIO_GLOBAL_H
#define PORTFOLIO_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(PORTFOLIO_LIBRARY)
#  define PORTFOLIOSHARED_EXPORT Q_DECL_EXPORT
#else
#  define PORTFOLIOSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // PORTFOLIO_GLOBAL_H
