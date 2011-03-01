//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#ifndef ___PlatformTypes_h___
#define ___PlatformTypes_h___

#include <QByteArray>
#include <time.h>

#ifdef WIN32
typedef __int64 gint64;
typedef unsigned __int64 guint64;
#else
typedef qint64 gint64;
typedef quint64 guint64;
#endif // WIN32

typedef unsigned int guint;
typedef int gint;
typedef unsigned char gbyte;
typedef unsigned short gushort;

typedef unsigned long gunichar;
typedef unsigned short guint16;
typedef short gint16;
typedef char gchar;
typedef unsigned char guchar;

typedef gushort gwchar;

typedef float gfloat;

#ifdef WIN32
// Use non deprecated function in VS 2005
#pragma warning(disable : 4996)
#endif // WIN32

#define UNUSED(x) (void)x;

#endif // ___PlatformTypes_h___
