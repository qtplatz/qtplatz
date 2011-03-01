//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#ifndef __JTempFileStream_h__
#define __JTempFileStream_h__

#include "JFileStream.h"
#include <QString>

///
/// Stream for temp file storage reading and writing
///

class JSTREAM_EXPORT JTempFileStream : public JFileStream
{
public:

	JTempFileStream();
	JTempFileStream( const QString &fileName );
	virtual ~JTempFileStream();

	virtual bool open( const QString& fileName );
	void setAutoDelete( bool flag );

protected:

	bool autoDelete_;
	void cleanUp();

private:

	JTempFileStream ( const JTempFileStream& rhs );			// cannot be copied
	JTempFileStream& operator = ( const JTempFileStream& rhs );	// nor assigned
};

//static const QString& generateUnqueFileName( const QString &filePath, QString &uniqueFileName );

#endif //__JTempFileStream_h__

