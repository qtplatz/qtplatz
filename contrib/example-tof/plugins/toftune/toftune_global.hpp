#ifndef tofTUNE_GLOBAL_HPP
#define tofTUNE_GLOBAL_HPP

#include <QtCore/QtGlobal>

#if defined(tofTUNE_LIBRARY)
#  define tofTUNESHARED_EXPORT Q_DECL_EXPORT
#else
#  define tofTUNESHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // tofTUNE_GLOBAL_H

