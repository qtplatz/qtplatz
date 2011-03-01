//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#ifndef __EfsFile_h__
#define __EfsFile_h__

#include "PlatformTypes.h"
#include <QFile>

///
/// Class EfsFile
///

class EfsFile
{
public:

	virtual bool open( const wchar_t * filename ) = 0;
	virtual int read( char *buffer, int size ) = 0;
	virtual int write( const char *buffer, int size ) = 0;
	virtual gint64 seek( gint64 size ) = 0;
	virtual gint64 pos() = 0;
	virtual gint64 size() = 0;
	virtual void close() = 0;

	virtual bool resize( gint64 newSize ) = 0;
	virtual bool isEmpty() = 0;

	virtual void remove( const wchar_t * path ) = 0;
	virtual void rename( const wchar_t * from, const wchar_t * to ) = 0;

	// Get instance of EfsFile class for current OS (factory)
	static EfsFile* GetInstance();

};

#endif // __EfsFile_h__

