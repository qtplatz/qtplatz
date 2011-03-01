//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#ifndef __EfsHeader_h__
#define __EfsHeader_h__

#include "EfsFile.h"
#include "EfsDefs.h"
#include <QByteArray>

///
/// Class EfsHeader represents file header 
/// of the Efs with all necessary service 
/// information
///
/// offset:size - description
/// 0:8		signature
/// 8:4		version
/// 12:4	unmountedState
/// 16:EfsOverheadSize EfsOverhead structure
///

class EfsHeader
{
public:

	EfsHeader( EfsFile *fs );
	EfsHeader( EfsFile *fs, const EfsOverhead &info );

	// Methods
	void version( int ver );
	int version();

	void setSignature( char *sig, int sigSize );
	bool isTrueSignature(  char *sig, int sigSize  );

	void writeHeader();
	bool readHeader();

	void setMounted( bool state );
	bool mounted();

	// Set overhead info
	bool initBlockHeader( const EfsOverhead &info );

	EfsOverhead* overhead() const;
	EfsServiceBlock* info() const;

protected:

	EfsFile *fs_;
	//char serviceBlock[ EfsBlockSize + 1 ];
	QByteArray serviceBlock_;
};

#endif // __EfsHeader_h__

