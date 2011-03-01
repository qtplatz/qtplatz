//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#ifndef __JUtils_h__
#define __JUtils_h__

#include "time.h"

#ifdef WIN32
# define JSTREAM_EXPORT __declspec(dllexport) 
#else
# define JSTREAM_EXPORT
#endif

///
/// Jlobal utility methods
///

class JSTREAM_EXPORT JUtils
{
public:

	static time_t filetimeToUnixTime( int* fileTime );

};

#endif // __JUtils_h__
