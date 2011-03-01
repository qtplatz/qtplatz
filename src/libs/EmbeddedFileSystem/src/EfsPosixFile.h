//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#ifndef __EfsPosixFile_h__
#define __EfsPosixFile_h__

#ifndef WIN32

#include <stdio.h>

#include "EfsFile.h"

/// Class EfsPosixFile

class EfsPosixFile : public EfsFile
{
public:

	EfsPosixFile();
	virtual ~EfsPosixFile();

	virtual bool open( const unsigned short *filename );
	virtual int read( char *buffer, int buffer_size );
	virtual int write( const char *buffer, int buffer_size );
	virtual gint64 seek( gint64 size );
	virtual gint64 pos();
	virtual void close();

	virtual bool resize( gint64 newSize );
	virtual gint64 size();
	virtual bool isEmpty();

	virtual void remove( const unsigned short *path );
	virtual void rename( const unsigned short *from, const unsigned short *to );

private:

	int fd_;
};

#endif // WIN32

#endif // __EfsPosixFile_h__

