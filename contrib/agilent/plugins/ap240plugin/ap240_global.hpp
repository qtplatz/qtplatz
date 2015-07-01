#ifndef AP240_GLOBAL_HPP
#define AP240_GLOBAL_HPP

#include <QtGlobal>

#if defined(AP240_LIBRARY)
#  define AP240SHARED_EXPORT Q_DECL_EXPORT
#else
#  define AP240SHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // AP240_GLOBAL_H

