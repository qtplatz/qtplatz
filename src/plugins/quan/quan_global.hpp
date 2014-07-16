#ifndef QUAN_GLOBAL_HPP
#define QUAN_GLOBAL_HPP

#include <QtGlobal>

#if defined(QUAN_LIBRARY)
#  define QUANSHARED_EXPORT Q_DECL_EXPORT
#else
#  define QUANSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // QUAN_GLOBAL_H

