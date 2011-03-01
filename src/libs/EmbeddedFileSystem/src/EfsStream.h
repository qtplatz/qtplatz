//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#ifndef __EfsStream_h__
#define __EfsStream_h__

#include "JStream.h"
#include "Efs.h"

///
/// Class EfsStream
///

class EfsStream : public JStream
{
public:

	EfsStream();
	EfsStream( Efs *afs );
	virtual ~EfsStream();

	void setFS( Efs *afs );

	bool open( const char *path, int access, int createDisp );
	bool isOpen();

	// Read methods
	virtual gint64 read( char* pBuffer, gint64 bufLen );

	virtual bool atEnd() const;
	virtual void close();
	virtual bool seek( gint64 position );
	virtual gint64 size();
	virtual gint64 pos() const;

	// Write methods
	virtual gint64 write( const char* pBuffer, gint64 bufLen );

	virtual void reset() {};
protected:

	Efs *efs_;
	int fd_;
};

#endif // __EfsStream_h__
