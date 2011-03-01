//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#ifndef __EfsWin32File_h__
#define __EfsWin32File_h__

#include "EfsFile.h"

#ifdef WIN32

#define _WIN32_WINNT 0x0400
#define _WIN32_WINDOWS 0x0400
#define WIN32_LEAN_AND_MEAN

#include <windows.h>

///
/// Class EfsWin32File
///

class EfsWin32File : public EfsFile
{
public:

	EfsWin32File();
	~EfsWin32File();

	virtual bool open( const wchar_t * filepath );
	virtual int read( char *buffer, int size );
	virtual int write( const char *buffer, int size );
	virtual gint64 seek( gint64 size );
	virtual gint64 pos();
	virtual void close();

	virtual bool resize( gint64 newSize );
	virtual gint64 size();
	virtual bool isEmpty();

	virtual void remove( const wchar_t *path );
	virtual void rename( const wchar_t *from, const wchar_t *to );

private:

	HANDLE hFile_;

};

#endif // WIN32
#endif // __EfsWin32File_h__
